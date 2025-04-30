#pragma once
#include <Text/liquid_text.h>
#include <stdbool.h>
#include <stdio.h> // <-- for Debuging

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
void liquidEventListener(LiquidEvent *event, bool* isRunning, int* mouseX, int* mouseY);