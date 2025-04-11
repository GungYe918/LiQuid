#include "liquid_internal.h"
#include "../include/liquid_event.h"
#include "platform/platform.h"

bool liquid_poll_event(LiquidEvent *event) {
    return platform_poll_event(event);
}