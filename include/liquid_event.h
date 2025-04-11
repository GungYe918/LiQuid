#pragma once
#include <stdbool.h>

typedef enum {
    LIQUID_EVENT_NONE,
    LIQUID_EVENT_QUIT,
    LIQUID_EVENT_KEYDOWN,
    LIQUID_EVENT_MOUSEMOVE
} LiquidEventType;

typedef struct {
    LiquidEventType type;

    union 
    {
        struct {    int keycode;    } key;
        struct {    int x, y;       } mouse;
    };
    
} LiquidEvent;



bool liquid_poll_event(LiquidEvent *event);