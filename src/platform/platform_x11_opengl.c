#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include <platform/platform.h>
#include <liquid_event.h>
#include <liquid_graphics.h>
#include "../liquid_internal.h"

// OpenGL 프레임버퍼 확장 함수 포인터
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;

// OpenGL 상수 (glext.h에 없는 경우를 대비)
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0 0x8CE0
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#endif

static Display* display = NULL;
static Window window;
static GLXContext glContext;
static Atom wmDeleteMessage;
static Pixmap pixmap = 0;
static GLXPixmap glxPixmap = 0;

static bool resizePending = false;
static int pendingWidth = 0;
static int pendingHeight = 0;

int logicalWidth = 0;
int logicalHeight = 0;

extern uint32_t clearColor;

static int width = 0;
static int height = 0;

static GLuint texture = 0;
static GLuint framebufferObj = 0;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);



// 더블 버퍼링을 위한 프레임버퍼 설정
static void create_framebuffer() {
    // 기존 텍스처와 프레임버퍼 정리
    if (framebufferObj) {
        glDeleteFramebuffers(1, &framebufferObj);
        framebufferObj = 0;
    }
    
    if (texture) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
    
    // 텍스처 생성
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // 프레임버퍼 생성
    glGenFramebuffers(1, &framebufferObj);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferObj);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    // 프레임버퍼 상태 확인
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "프레임버퍼 생성 실패: %d\n", status);
    }
    
    // 기본 프레임버퍼로 돌아가기
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// OpenGL 확장 함수 로드 헬퍼
static void* get_gl_proc_address(const char* name) {
    void* p = (void*)glXGetProcAddress((const GLubyte*)name);
    if (!p) {
        fprintf(stderr, "Failed to load GL function: %s\n", name);
    }
    return p;
}

// OpenGL 확장 함수 초기화
static bool init_gl_extensions() {
    
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)get_gl_proc_address("glGenFramebuffers");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)get_gl_proc_address("glBindFramebuffer");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)get_gl_proc_address("glFramebufferTexture2D");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)get_gl_proc_address("glCheckFramebufferStatus");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)get_gl_proc_address("glDeleteFramebuffers");
    
    // 각 함수 포인터 검증
    if (!glGenFramebuffers || !glBindFramebuffer || !glFramebufferTexture2D || 
        !glCheckFramebufferStatus || !glDeleteFramebuffers) {
        fprintf(stderr, "Failed to load required OpenGL extension functions\n");
        return false;
    }
    
    return true;
}

bool platform_init(int w, int h, const char* title) {
    width = w;
    height = h;
    logicalWidth = w;
    logicalHeight = h;

    // framebuffer 메모리 할당 전 NULL 체크 추가
    if (framebuffer) {
        free(framebuffer);
    }
    
    framebuffer = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    if (!framebuffer) {
        fprintf(stderr, "Failed to allocate framebuffer\n");
        return false;
    }
    memset(framebuffer, 0, width * height * sizeof(uint32_t));

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X display\n");
        return false;
    }

    // GLX 버전 확인
    int glx_major, glx_minor;
    if (!glXQueryVersion(display, &glx_major, &glx_minor) || 
       ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1)) {
        fprintf(stderr, "Invalid GLX version: %d.%d\n", glx_major, glx_minor);
        return false;
    }

    static int visual_attribs[] = {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        None
    };

    int screen = DefaultScreen(display);

    // FBConfigs 가져오기
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(display, screen, visual_attribs, &fbcount);
    if (!fbc) {
        fprintf(stderr, "Failed to retrieve framebuffer configurations\n");
        return false;
    }

    // 최적의 FBConfig 선택
    int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
    for (int i = 0; i < fbcount; i++) {
        XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[i]);
        if (vi) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);

            if (best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
                best_fbc = i;
                best_num_samp = samples;
            }
            if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp) {
                worst_fbc = i;
                worst_num_samp = samples;
            }
            XFree(vi);
        }
    }

    GLXFBConfig bestFbc = fbc[best_fbc];
    XFree(fbc);

    // 선택한 FBConfig에서 Visual 가져오기
    XVisualInfo *vi = glXGetVisualFromFBConfig(display, bestFbc);
    if (!vi) {
        fprintf(stderr, "Failed to create appropriate visual for window\n");
        return false;
    }

    // 윈도우 설정
    XSetWindowAttributes swa;
    swa.colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
    swa.border_pixel = 0;
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask | PointerMotionMask;

    window = XCreateWindow(
        display,
        RootWindow(display, vi->screen),
        0, 0, width, height,
        0,                          // 테두리 너비
        vi->depth,                  // 색상 깊이
        InputOutput,                // 윈도우 클래스
        vi->visual,                 // Visual
        CWBorderPixel | CWColormap | CWEventMask,
        &swa
    );

    XStoreName(display, window, title);

    wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    XMapWindow(display, window);

    // GLX 컨텍스트 속성
    int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
        None
    };

    // 최신 컨텍스트 생성 함수 가져오기
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
        glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");

    // GLX 컨텍스트 생성
    if (glXCreateContextAttribsARB) {
        glContext = glXCreateContextAttribsARB(display, bestFbc, 0, True, context_attribs);
    } else {
        // 폴백: 레거시 컨텍스트 생성
        glContext = glXCreateContext(display, vi, NULL, GL_TRUE);
    }

    XFree(vi);

    if (!glContext) {
        fprintf(stderr, "Failed to create GLX context\n");
        return false;
    }

    // 컨텍스트 활성화
    if (!glXMakeCurrent(display, window, glContext)) {
        fprintf(stderr, "Failed to make context current\n");
        return false;
    }
    
    // OpenGL 초기 설정
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // OpenGL 확장 함수 로드
    if (!init_gl_extensions()) {
        fprintf(stderr, "Failed to initialize OpenGL extensions\n");
        return false;
    }
    
    // 프레임버퍼 및 텍스처 초기 설정
    create_framebuffer();

    return true;
}

static void set_clear_color_from_u32(uint32_t color) {
    float r = ((color >> 24) & 0xFF) / 255.0f;
    float g = ((color >> 16) & 0xFF) / 255.0f;
    float b = ((color >> 8)  & 0xFF) / 255.0f;
    float a = ((color)       & 0xFF) / 255.0f;
    glClearColor(r, g, b, a);
}

void platform_draw(const uint32_t* buffer) {
    if (!buffer || !display || !texture) return;  // texture NULL 체크 추가

    // 텍스처에 프레임버퍼 데이터 업로드
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
    
    // 화면 지우기
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // 추가: 검은색으로 명확하게 지정
    set_clear_color_from_u32(clearColor);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // 프레임버퍼 텍스처 그리기
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // 추가: 흰색으로 그리기
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f((float)width, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f((float)width, (float)height);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, (float)height);
    glEnd();
    
    // 바로 버퍼 스왑을 호출하도록 변경
    glXSwapBuffers(display, window);
}

static bool resize_image(int new_width, int new_height) {
    if (framebuffer) {
        free(framebuffer);
        framebuffer = NULL;
    }

    framebuffer = malloc(new_width * new_height * sizeof(uint32_t));
    if (!framebuffer) return false;
    memset(framebuffer, 0, new_width * new_height * sizeof(uint32_t));

    width = new_width;
    height = new_height;
    logicalWidth = width;
    logicalHeight = height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    create_framebuffer();

    return true;
}



void platform_shutdown() {
    if (framebuffer) {
        free(framebuffer);
        framebuffer = NULL;
    }
    
    // OpenGL 리소스 정리
    if (framebufferObj) {
        glDeleteFramebuffers(1, &framebufferObj);
    }
    
    if (texture) {
        glDeleteTextures(1, &texture);
    }
    
    if (glxPixmap) {
        glXDestroyGLXPixmap(display, glxPixmap);
    }
    
    if (pixmap) {
        XFreePixmap(display, pixmap);
    }
    
    // GLX 컨텍스트 정리
    if (display && glContext) {
        glXMakeCurrent(display, None, NULL);
        glXDestroyContext(display, glContext);
    }
    
    // X11 리소스 정리
    if (display && window) {
        XDestroyWindow(display, window);
    }
    
    if (display) {
        XCloseDisplay(display);
    }
}

bool platform_poll_event(LiquidEvent* event) {
    if (!event || !display) return false;

    while (XPending(display)) {
        XEvent e;
        XNextEvent(display, &e);

        switch (e.type) {
            case ClientMessage:
                if ((Atom)e.xclient.data.l[0] == wmDeleteMessage) {
                    event->type = LIQUID_EVENT_QUIT;
                    return true;
                }
                break;

            case ConfigureNotify:
                if (e.xconfigure.width != width || e.xconfigure.height != height) {
                    resizePending = true;
                    pendingWidth = e.xconfigure.width;
                    pendingHeight = e.xconfigure.height;
                }
                break;

            case KeyPress:
                event->type = LIQUID_EVENT_KEY_DOWN;
                event->key.keycode = e.xkey.keycode;
                {
                    char buf[8] = {0};
                    KeySym keysym;
                    XLookupString(&e.xkey, buf, sizeof(buf), &keysym, NULL);
                    event->key.character = buf[0];
                }
                return true;

            case KeyRelease:
                event->type = LIQUID_EVENT_KEY_UP;
                event->key.keycode = e.xkey.keycode;
                return true;

            case ButtonPress:
                event->type = LIQUID_EVENT_MOUSE_DOWN;
                event->mouse.x = e.xbutton.x;
                event->mouse.y = e.xbutton.y;
                event->mouse.button = e.xbutton.button;
                return true;

            case ButtonRelease:
                event->type = LIQUID_EVENT_MOUSE_UP;
                event->mouse.x = e.xbutton.x;
                event->mouse.y = e.xbutton.y;
                event->mouse.button = e.xbutton.button;
                return true;

            case MotionNotify:
                event->type = LIQUID_EVENT_MOUSE_MOVE;
                event->mouse.x = e.xmotion.x;
                event->mouse.y = e.xmotion.y;
                return true;
        }
    }

    if (resizePending) {
        if (!resize_image(pendingWidth, pendingHeight)) {
            event->type = LIQUID_EVENT_ERROR;
            return true;
        }

        resizePending = false;
        event->type = LIQUID_EVENT_RESIZE;
        event->mouse.x = width;
        event->mouse.y = height;
        return true;
    }

    return false;
}

void platform_present() {
    if (!display || !window) {
        fprintf(stderr, "Cannot present: display or window is NULL\n");
        return;
    }
    
    // OpenGL 상태를 초기 상태로 복원
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // 버퍼 스왑
    glXSwapBuffers(display, window);
}