#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include <platform/platform.h>
#include "../liquid_internal.h"

static Display* display = NULL;
static Window window;
static GC graphicsContext;
static XImage* image = NULL;
static Atom wmDeleteMessage;
static int width, height;

int logicalWidth    = 0;
int logicalHeight   = 0;

static bool resizePending = false;
static int pendingWidth = 0;
static int pendingHeight = 0;

__attribute__((unused)) 
    static uint32_t* framebufferRef = NULL;     // BACKEND=fb일때만 사용


bool platform_init(int w, int h, const char* title) {
        logicalWidth = width = w;
        logicalHeight = height = h;
    
        framebuffer = malloc(width * height * sizeof(uint32_t));
        if (!framebuffer) {
            fprintf(stderr, "Failed to allocate framebuffer (%d x %d)\n", width, height);
            return false;
        }
    
        display = XOpenDisplay(NULL);
        if (!display) {
            fprintf(stderr, "Unable to open X display\n");
            return false;
        }
    
        int screen = DefaultScreen(display);
        window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, width, height,
                                     1, BlackPixel(display, screen), WhitePixel(display, screen));
    
        XStoreName(display, window, title);
        XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask | PointerMotionMask);
        graphicsContext = XCreateGC(display, window, 0, NULL);
    
        wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display, window, &wmDeleteMessage, 1);
    
        XMapWindow(display, window);
    
        void* imageData = malloc(width * height * 4);
        if (!imageData) {
            fprintf(stderr, "Failed to allocate memory for image data\n");
            return false;
        }
    
        int bytes_per_line = width * 4;
    
        image = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen),
                             ZPixmap, 0, imageData, width, height, 32, bytes_per_line);
    
        if (!image) {
            free(imageData);
            fprintf(stderr, "Failed to create XImage\n");
            return false;
        }
    
        return true;
}

void platform_shutdown() {
    if (image) {
        free(image->data);
        image->data = NULL;
        XDestroyImage(image);
        image = NULL;
    }

    if (display) {
        XDestroyWindow(display, window);
        XCloseDisplay(display);
        display = NULL;
    }
}

void platform_draw(const uint32_t* framebuffer) {
    if (!image || !framebuffer) return;

    memcpy(image->data, framebuffer, width * height * 4);
    XPutImage(display, window, graphicsContext, image, 0, 0, 0, 0, width, height);
    XFlush(display);
}

// 윈도우 크기가 변경되었을 때 이미지 재생성을 위한 함수
static bool resize_image(int new_width, int new_height) {
    if (image) {
        free(image->data);
        image->data = NULL;
        XDestroyImage(image);
        image = NULL;
    }

    if (framebuffer) {
        free(framebuffer);
        framebuffer = NULL;  // 새 할당 전 NULL 처리
    }
    
    // 새 크기로 메모리 재할당
    framebuffer = malloc(new_width * new_height * sizeof(uint32_t));
    if (!framebuffer) {
        fprintf(stderr, "Failed to reallocate framebuffer\n");
        return false;
    }
    memset(framebuffer, 0, new_width * new_height * sizeof(uint32_t));


    void* newImageData = malloc(new_width * new_height * 4);
    if (!newImageData) {
        fprintf(stderr, "Failed to reallocate image data for size %d x %d\n", new_width, new_height);
        return false;
    }
    
    int bytes_per_line = new_width * 4;
    
    image = XCreateImage(
        display,
        DefaultVisual(display, DefaultScreen(display)),
        DefaultDepth(display, DefaultScreen(display)),
        ZPixmap,
        0,
        newImageData,
        new_width,
        new_height,
        32,
        bytes_per_line
    );
    
    if (!image) {
        fprintf(stderr, "Failed to create new XImage after resize\n");
        free(newImageData);
        return false;
    }
    
    width = new_width;
    height = new_height;
    logicalWidth = width;
    logicalHeight = height;
    
    return true;
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

            case ConfigureNotify:
                if (e.xconfigure.width != width || e.xconfigure.height != height) {
                    // resize 즉시 처리 대신 예약
                    resizePending = true;
                    pendingWidth = e.xconfigure.width;
                    pendingHeight = e.xconfigure.height;
                }
                break;

            case ClientMessage:
                if ((Atom)e.xclient.data.l[0] == wmDeleteMessage) {
                    event->type = LIQUID_EVENT_QUIT;
                    return true;
                }
                break;

            default:
                break;
        }
    }

    // 이벤트 큐가 비었을 때 resize를 실제로 처리
    if (resizePending) {
        if (!resize_image(pendingWidth, pendingHeight)) {
            event->type = LIQUID_EVENT_ERROR;
            return true;
        }

        resizePending = false;
        event->type = LIQUID_EVENT_RESIZE;
        event->mouse.x = pendingWidth;
        event->mouse.y = pendingHeight;
        return true;
    }

    return false;
}