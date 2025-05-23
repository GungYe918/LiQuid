#include "liquid_internal.h"
#include "../include/liquid_core.h"
#include "platform/platform.h"
//#include "platform/platform_switch.h"

extern uint32_t* framebuffer;

bool liquid_init(int width, int height, const char *title) {
    return platform_init(width, height, title);
}

void liquid_shutdown(void) {
    platform_shutdown();
}