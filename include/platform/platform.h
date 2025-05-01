#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../../include/liquid.h"

extern int logicalWidth;
extern int logicalHeight;

bool platform_init(int width, int height, const char *title);
void platform_shutdown(void);
bool platform_poll_event(LiquidEvent *event);
void platform_draw(const uint32_t *framebuffer);