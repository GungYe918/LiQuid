// platform_wayland.c
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <platform/platform.h>

static struct wl_display* display = NULL;
static struct wl_registry* registry = NULL;
static struct wl_compositor* compositor = NULL;
static struct wl_surface* surface = NULL;
static struct wl_shm* shm = NULL;
static struct wl_shm_pool* shm_pool = NULL;
static struct wl_buffer* buffer = NULL;

static struct xdg_wm_base* wm_base = NULL;
static struct xdg_surface* xdg_surface = NULL;
static struct xdg_toplevel* xdg_toplevel = NULL;

static uint32_t* framebuffer = NULL;
static int width = 0;
static int height = 0;

#ifndef MFD_CLOEXEC
#define MFD_CLOEXEC 0x0001U
#include <sys/syscall.h>
static int memfd_create(const char *name, unsigned int flags) {
    return syscall(SYS_memfd_create, name, flags);
}
#endif

static void handle_global(void* data, struct wl_registry* registry,
                          uint32_t name, const char* interface, uint32_t version) {
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, "wl_shm") == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
}

static void xdg_wm_base_ping(void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = NULL
};

static const struct xdg_wm_base_listener wm_base_listener = {
    .ping = xdg_wm_base_ping
};

bool platform_init(int w, int h, const char* title) {
    width = w;
    height = h;

    display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Unable to connect to Wayland display\n");
        return false;
    }

    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display);

    if (!compositor || !shm || !wm_base) {
        fprintf(stderr, "Missing required Wayland interfaces\n");
        return false;
    }

    wl_display_roundtrip(display);
    xdg_wm_base_add_listener(wm_base, &wm_base_listener, NULL);

    surface = wl_compositor_create_surface(compositor);
    xdg_surface = xdg_wm_base_get_xdg_surface(wm_base, surface);
    xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
    xdg_toplevel_set_title(xdg_toplevel, title ? title : "Liquid Wayland");

    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    int stride = width * 4;
    int size = stride * height;
    int fd = memfd_create("wayland-buffer", MFD_CLOEXEC);
    if (fd < 0) {
        perror("memfd_create");
        return false;
    }
    ftruncate(fd, size);

    framebuffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (framebuffer == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return false;
    }

    shm_pool = wl_shm_create_pool(shm, fd, size);
    buffer = wl_shm_pool_create_buffer(shm_pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);

    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage_buffer(surface, 0, 0, width, height);
    wl_surface_commit(surface);

    return true;
}

void platform_shutdown() {
    if (framebuffer) {
        munmap(framebuffer, width * height * 4);
        framebuffer = NULL;
    }
    if (buffer) wl_buffer_destroy(buffer);
    if (shm_pool) wl_shm_pool_destroy(shm_pool);
    if (xdg_toplevel) xdg_toplevel_destroy(xdg_toplevel);
    if (xdg_surface) xdg_surface_destroy(xdg_surface);
    if (surface) wl_surface_destroy(surface);
    if (wm_base) xdg_wm_base_destroy(wm_base);
    if (display) wl_display_disconnect(display);
    display = NULL;
}

void platform_draw(const uint32_t* src_framebuffer) {
    if (!framebuffer || !src_framebuffer) return;
    memcpy(framebuffer, src_framebuffer, width * height * 4);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage_buffer(surface, 0, 0, width, height);
    wl_surface_commit(surface);
    wl_display_flush(display);
}

bool platform_poll_event(LiquidEvent* event) {
    wl_display_dispatch_pending(display);
    event->type = LIQUID_EVENT_NONE;
    return false;
}