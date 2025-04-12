/*
#include "liquid_internal.h"
#include "../include/liquid.h"
#include "platform/platform.h"

uint32_t framebuffer[LIQUID_WIDTH * LIQUID_HEIGHT];

bool liquid_init(int width, int height, const char *title) {
    return platform_init(width, height, title);
}

void liquid_shutdown(void) {
    platform_shutdown();
}

void liquid_clear(uint32_t color) {
    for (int i = 0; i < LIQUID_WIDTH * LIQUID_HEIGHT; ++i) {
        framebuffer[i] = color;
    }
}

void liquid_draw_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= LIQUID_WIDTH || y < 0 || y >= LIQUID_HEIGHT) return;
    framebuffer[y * LIQUID_WIDTH + x] = color;
}

void liquid_present(void) {
    platform_draw(framebuffer);
}

bool liquid_poll_event(LiquidEvent *event) {
    return platform_poll_event(event);
}

*/