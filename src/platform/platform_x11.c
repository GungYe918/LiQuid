#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include <platform/platform.h>

static Display* display = NULL;
static Window window;
static GC graphicsContext;
static XImage* image = NULL;
static Atom wmDeleteMessage;
static int width, height;

__attribute__((unused)) 
    static uint32_t* framebufferRef = NULL;     // BACKEND=fb일때만 사용

bool platform_init(int w, int h, const char* title) {
    width = w;
    height = h;

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open X display\n");
        return false;
    }

    int screen = DefaultScreen(display);
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

    image = XCreateImage(
        display,
        DefaultVisual(display, screen),
        DefaultDepth(display, screen),
        ZPixmap,
        0,
        malloc(width * height * 4),
        width,
        height,
        32,
        0
    );

    if (!image) {
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