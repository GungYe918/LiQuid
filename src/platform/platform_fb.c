#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <platform/platform.h>
#include "../liquid_internal.h"  // <- 해상도 정의 가져오기

int logicalWidth    = 0;
int logicalHeight   = 0;

static int fb_fd = -1;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static uint8_t* fb_ptr = NULL;
static int fb_size = 0;
static int bytes_per_pixel = 0;

bool platform_init(int w, int h, const char* title) {
    (void)w;
    (void)h;
    (void)title;

    logicalWidth  = w;
    logicalHeight = h;

    fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        perror("Error: cannot open framebuffer device");
        return false;
    }

    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo)) {
        perror("Error reading fixed information");
        return false;
    }

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error reading variable information");
        return false;
    }

    bytes_per_pixel = vinfo.bits_per_pixel / 8;
    fb_size = finfo.line_length * vinfo.yres;

    if (bytes_per_pixel != 4 && bytes_per_pixel != 2) {
        fprintf(stderr, "Unsupported framebuffer format: %d bpp\n", vinfo.bits_per_pixel);
        return false;
    }

    fb_ptr = (uint8_t*)mmap(0, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_ptr == MAP_FAILED) {
        perror("Error: mmap failed");
        return false;
    }

    printf("Framebuffer initialized: %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
    return true;
}

void platform_shutdown() {
    if (fb_ptr && fb_ptr != MAP_FAILED) {
        munmap(fb_ptr, fb_size);
        fb_ptr = NULL;
    }
    if (fb_fd != -1) {
        close(fb_fd);
        fb_fd = -1;
    }
}

void platform_draw(const uint32_t* framebuffer_input) {
    if (!fb_ptr || !framebuffer_input) return;

    int x, y;
    for (y = 0; y < LIQUID_HEIGHT; ++y) {
        uint8_t* dst = fb_ptr + y * finfo.line_length;
        const uint32_t* src = framebuffer_input + y * LIQUID_WIDTH;

        for (x = 0; x < LIQUID_WIDTH; ++x) {
            uint32_t pixel = src[x];

            if (bytes_per_pixel == 4) {
                dst[x * 4 + 0] = pixel & 0xFF;        // B
                dst[x * 4 + 1] = (pixel >> 8) & 0xFF; // G
                dst[x * 4 + 2] = (pixel >> 16) & 0xFF;// R
                dst[x * 4 + 3] = 0xFF;                // A
            } else if (bytes_per_pixel == 2) {
                // RGB565로 변환
                uint16_t r = (pixel >> 19) & 0x1F;
                uint16_t g = (pixel >> 10) & 0x3F;
                uint16_t b = (pixel >> 3)  & 0x1F;
                uint16_t rgb565 = (r << 11) | (g << 5) | b;
                ((uint16_t*)dst)[x] = rgb565;
            }
        }
    }
}

bool platform_poll_event(LiquidEvent* event) {
    event->type = LIQUID_EVENT_NONE;
    return false; // 향후 evdev로 입력 구현 가능
}
