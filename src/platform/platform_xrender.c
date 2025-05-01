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

    // 1. GPU-friendly Pixmap 생성 (depth = 32)
    Pixmap pixmap = XCreatePixmap(display, window, width, height, 32);

    // 2. GC 생성 (픽셀 데이터를 GPU에 그리기 위한 그래픽 컨텍스트)
    GC temp_gc = XCreateGC(display, pixmap, 0, NULL);

    // 3. XImage 생성 (이건 CPU buffer지만, XPutImage 후 GPU 메모리에 올라감)
    XImage* img = XCreateImage(
        display,
        CopyFromParent, // 정확한 Visual은 Pixmap엔 불필요
        32,
        ZPixmap,
        0,
        (char*)malloc(width * height * 4),  // RGBA
        width,
        height,
        32,
        0
    );

    if (!img || !img->data) {
        fprintf(stderr, "XImage creation failed\n");
        if (img) XDestroyImage(img);
        XFreePixmap(display, pixmap);
        return;
    }

    // 4. CPU에서 RGBA 데이터 복사
    memcpy(img->data, framebuffer, width * height * 4);

    // 5. GPU 메모리에 XImage 업로드 (여기서 드라이버가 GPU에 upload 가능)
    XPutImage(display, pixmap, temp_gc, img, 0, 0, 0, 0, width, height);

    // 6. Pixmap → Picture (GPU 텍스처와 유사한 개념)
    XRenderPictFormat* src_format = XRenderFindStandardFormat(display, PictStandardARGB32);
    if (!src_format) {
        fprintf(stderr, "Failed to find ARGB32 format\n");
        XDestroyImage(img);
        XFreeGC(display, temp_gc);
        XFreePixmap(display, pixmap);
        return;
    }

    Picture src_picture = XRenderCreatePicture(display, pixmap, src_format, 0, NULL);

    // 7. Composite → GPU 블렌딩 (하드웨어 가속)
    XRenderComposite(
        display,
        PictOpSrc,         // 덮어쓰기
        src_picture,       // 소스
        None,              // 마스크 없음
        picture,           // 대상 윈도우
        0, 0,              // src x, y
        0, 0,              // mask x, y
        0, 0,              // dst x, y
        width, height
    );

    // 8. 정리
    XRenderFreePicture(display, src_picture);
    XFreeGC(display, temp_gc);
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