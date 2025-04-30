#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrender.h>

#include <stdlib.h>
#include <liquid_graphics.h>

#include <stdio.h> // for debugging

static Display* display = NULL;
static Window window;
static GC graphicsContext;
static Atom wmDeleteMessage;
static XImage* image = NULL;

static Picture picture;
static XRenderPictFormat* format;

static int width = 0;
static int height = 0;

bool platform_init(int w, int h, const char* title) {
    width = w;
    height = h;

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X display\n");
        return false;
    }

    int screen = DefaultScreen(display);
    Visual* visual = DefaultVisual(display, screen);
    int depth = DefaultDepth(display, screen);
    
    // 현재 디스플레이가 32비트 ARGB를 지원하는지 확인
    XRenderPictFormat* pictFormat = XRenderFindStandardFormat(display, PictStandardARGB32);
    if (!pictFormat) {
        fprintf(stderr, "Display does not support 32-bit ARGB format\n");
        // 여기서 대체 포맷을 시도할 수 있습니다
    }

    window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        0, 0, width, height,
        1,
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );

    XStoreName(display, window, title);
    XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask | PointerMotionMask);
    graphicsContext = XCreateGC(display, window, 0, NULL);

    wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    XMapWindow(display, window);

    // 창 속성 가져오기
    XWindowAttributes attrs;
    XGetWindowAttributes(display, window, &attrs);
    
    // 창의 Visual에 맞는 XRender 포맷 찾기
    format = XRenderFindVisualFormat(display, attrs.visual);
    if (!format) {
        fprintf(stderr, "Failed to find matching visual format for window\n");
        return false;
    }

    XRenderPictureAttributes pa = {0};
    picture = XRenderCreatePicture(display, window, format, 0, &pa);

    // image는 현재 창의 Visual과 깊이에 맞게 생성
    image = XCreateImage(
        display,
        attrs.visual,
        attrs.depth,
        ZPixmap,
        0,
        malloc(width * height * 4),  // 4바이트 할당 (RGBA)
        width,
        height,
        32,  // bitmap_pad
        0    // bytes_per_line (0으로 설정하면 자동 계산)
    );

    if (!image || !image->data) {
        fprintf(stderr, "Failed to create XImage\n");
        return false;
    }

    // 디버그 정보 출력
    fprintf(stderr, "Window created with depth: %d, bits_per_pixel: %d\n", 
           attrs.depth, image->bits_per_pixel);

    return true;
}

void platform_draw(const uint32_t* framebuffer) {
    if (!framebuffer || !display) return;

    // 창의 깊이와 시각적 정보를 올바르게 가져옵니다
    int screen = DefaultScreen(display);
    Visual* visual = DefaultVisual(display, screen);
    int depth = DefaultDepth(display, screen);

    // 현재 창의 속성을 가져옵니다
    XWindowAttributes attrs;
    XGetWindowAttributes(display, window, &attrs);

    // 창의 깊이에 맞게 Pixmap 생성 (무조건 32가 아니라 실제 깊이 사용)
    Pixmap pixmap = XCreatePixmap(display, window, width, height, attrs.depth);

    // 창의 Visual과 같은 Visual을 사용하여 XImage 생성
    XImage* img = XCreateImage(
        display,
        attrs.visual,
        attrs.depth,
        ZPixmap,
        0,
        (char*)malloc(width * height * 4),  // 4바이트 할당 (RGBA)
        width,
        height,
        32,  // bitmap_pad
        0    // bytes_per_line (0으로 설정하면 자동 계산)
    );

    if (!img || !img->data) {
        fprintf(stderr, "Failed to create XImage\n");
        if (img) XDestroyImage(img);
        XFreePixmap(display, pixmap);
        return;
    }

    // framebuffer 데이터 복사
    memcpy(img->data, framebuffer, width * height * 4);

    // Pixmap에 이미지 업로드
    XPutImage(display, pixmap, graphicsContext, img, 0, 0, 0, 0, width, height);

    // Pixmap을 Picture로 변환
    // 여기서 올바른 포맷을 찾습니다
    XRenderPictFormat* pixmapFormat;
    
    if (attrs.depth == 32) {
        pixmapFormat = XRenderFindStandardFormat(display, PictStandardARGB32);
    } else if (attrs.depth == 24) {
        pixmapFormat = XRenderFindStandardFormat(display, PictStandardRGB24);
    } else {
        // 다른 깊이에 대한 처리
        pixmapFormat = XRenderFindVisualFormat(display, attrs.visual);
    }
    
    if (!pixmapFormat) {
        fprintf(stderr, "Failed to find pixmap format\n");
        XDestroyImage(img);
        XFreePixmap(display, pixmap);
        return;
    }

    Picture srcPicture = XRenderCreatePicture(display, pixmap, pixmapFormat, 0, NULL);

    // GPU 가속 합성 (Pixmap → Window)
    XRenderComposite(
        display,
        PictOpSrc,       // 완전히 덮어쓰기
        srcPicture,      // src
        None,            // mask
        picture,         // dst
        0, 0, 0, 0, 0, 0,
        width, height
    );

    // 리소스 정리
    XRenderFreePicture(display, srcPicture);
    XFreePixmap(display, pixmap);
    XDestroyImage(img);
}


void platform_present() {
    XFlush(display);
}

void platform_shutdown() {
    XRenderFreePicture(display, picture);
    XFreeGC(display, graphicsContext);
    XCloseDisplay(display);
}

bool platform_poll_event(LiquidEvent* event) {
    if (!event || !display) return false;
    
    while (XPending(display)) {
        XEvent e;
        XNextEvent(display, &e);

        switch (e.type) {
            case KeyPress:
                event->type = LIQUID_EVENT_KEY_DOWN;
                event->key.keycode = e.xkey.keycode;

                char buf[8] = {0};
                KeySym keysym;
                XLookupString(&e.xkey, buf, sizeof(buf), &keysym, NULL);
                event->key.character = buf[0];

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
            

            default:
                return false;
        }
    }

    return false;
}