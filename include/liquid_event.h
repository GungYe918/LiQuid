#pragma once
#include <stdbool.h>

typedef enum {
    LIQUID_EVENT_NONE,
    LIQUID_EVENT_QUIT,
    LIQUID_EVENT_KEY_DOWN,
    LIQUID_EVENT_KEY_UP,
    LIQUID_EVENT_MOUSE_MOVE,
    LIQUID_EVENT_MOUSE_DOWN,
    LIQUID_EVENT_MOUSE_UP,
} LiquidEventType;

typedef struct {
    LiquidEventType type;

    union {
        struct {
            int keycode;
            char character;
        } key;

        struct {
            int x, y;
            int button;
        } mouse;
    };
    
} LiquidEvent;



bool liquid_poll_event(LiquidEvent *event);