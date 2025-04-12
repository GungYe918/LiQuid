#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../../include/liquid.h"

bool platform_init(int width, int height, const char *title);
void platform_shutdown(void);
void platform_draw(const uint32_t *framebuffer);
bool platform_poll_event(LiquidEvent *event);