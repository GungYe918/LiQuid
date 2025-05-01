#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrender.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include <platform/platform.h>
#include <liquid_event.h>
#include "../liquid_internal.h"

static Display* display = NULL;
static Window window;
static GC graphicsContext;
static Atom wmDeleteMessage;
static XImage* image = NULL;

static Picture picture;
static XRenderPictFormat* format;

static Pixmap backBuffer = 0;
static Picture backPicture = 0;
static GC backGC = 0;

static bool resizePending = false;
static int pendingWidth = 0;
static int pendingHeight = 0;

int logicalWidth = 0;
int logicalHeight = 0;

static int width = 0;
static int height = 0;

static void create_back_buffer() {
    if (backPicture) {
        XRenderFreePicture(display, backPicture);
        backPicture = 0;
    }

    if (backGC) {
        XFreeGC(display, backGC);
        backGC = 0;
    }

    if (backBuffer) {
        XFreePixmap(display, backBuffer);
        backBuffer = 0;
    }

    // 안전한 Visual & Format 추출
    int screen = DefaultScreen(display);
    Visual* visual = DefaultVisual(display, screen);
    int depth = DefaultDepth(display, screen);

    backBuffer = XCreatePixmap(display, window, width, height, depth);
    backGC = XCreateGC(display, backBuffer, 0, NULL);

    // 반드시 Visual 기반으로 format을 추출해야 한다
    XRenderPictFormat* pixmap_format = XRenderFindVisualFormat(display, visual);
    if (!pixmap_format) {
        fprintf(stderr, "Failed to find matching XRender format for Visual.\n");
        return;
    }

    backPicture = XRenderCreatePicture(display, backBuffer, pixmap_format, 0, NULL);
}


bool platform_init(int w, int h, const char* title) {
    width = w;
    height = h;
    logicalWidth = w;
    logicalHeight = h;

    framebuffer = malloc(width * height * sizeof(uint32_t));
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

    XWindowAttributes attrs;
    XGetWindowAttributes(display, window, &attrs);
    format = XRenderFindVisualFormat(display, attrs.visual);
    if (!format) {
        fprintf(stderr, "Failed to find matching XRender format for Visual.\n");
        return false;
    }

    XRenderPictureAttributes pa = {0};
    picture = XRenderCreatePicture(display, window, format, 0, &pa);
    if (!picture) {
        fprintf(stderr, "Failed to create XRender picture.\n");
        return false;
    }

    create_back_buffer();

    image = XCreateImage(
        display,
        attrs.visual,
        attrs.depth,
        ZPixmap,
        0,
        malloc(width * height * 4),
        width,
        height,
        32,
        width * 4
    );

    if (!image || !image->data) {
        fprintf(stderr, "Failed to create XImage\n");
        return false;
    }

    return true;
}


void platform_draw(const uint32_t* framebuffer) {
    if (!framebuffer || !display || !format) return;

    memcpy(image->data, framebuffer, width * height * 4);
    XPutImage(display, backBuffer, backGC, image, 0, 0, 0, 0, width, height);

    XRenderComposite(
        display,
        PictOpSrc,
        backPicture,
        None,
        picture,
        0, 0, 0, 0, 0, 0,
        width,
        height
    );

    
}

static bool resize_image(int new_width, int new_height) {
    if (framebuffer) {
        free(framebuffer);
        framebuffer = NULL;
    }

    framebuffer = malloc(new_width * new_height * sizeof(uint32_t));
    if (!framebuffer) return false;
    memset(framebuffer, 0, new_width * new_height * sizeof(uint32_t));

    if (image) {
        free(image->data);
        image->data = NULL;
        XDestroyImage(image);
        image = NULL;
    }

    // Visual과 Depth는 처음 생성된 window와 동일한 값을 사용해야 함
    XWindowAttributes attrs;
    XGetWindowAttributes(display, window, &attrs);

    image = XCreateImage(
        display,
        attrs.visual,
        attrs.depth,            
        ZPixmap, 0,
        malloc(new_width * new_height * 4),
        new_width, new_height,
        32, new_width * 4
    );

    if (!image || !image->data) return false;

    width = new_width;
    height = new_height;
    logicalWidth = width;
    logicalHeight = height;

    create_back_buffer();
    return true;
}

void platform_shutdown() {
    if (image) {
        free(image->data);
        image->data = NULL;
        XDestroyImage(image);
        image = NULL;
    }
    if (framebuffer) {
        free(framebuffer);
        framebuffer = NULL;
    }
    if (backPicture) XRenderFreePicture(display, backPicture);
    if (backGC) XFreeGC(display, backGC);
    if (backBuffer) XFreePixmap(display, backBuffer);
    XRenderFreePicture(display, picture);
    XFreeGC(display, graphicsContext);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

bool platform_poll_event(LiquidEvent* event) {
    if (!event || !display) return false;

    while (XPending(display)) {
        XEvent e;
        XNextEvent(display, &e);

        switch (e.type) {
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
    XFlush(display);
}
