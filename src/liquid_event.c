#include "liquid_internal.h"
#include "../include/liquid_event.h"
#include "platform/platform.h"

char inputBuffer[INPUT_BUFFER_SIZE] = {0};      // 문자열 누적 버퍼 (256 MAX)
int inputLength = 0;                            // 현재 입력된 문자 수

bool liquid_poll_event(LiquidEvent *event) {
    return platform_poll_event(event);
}

void liquidEventListener(
    LiquidEvent *event, bool* isRunning, 
    int* mouseX, int* mouseY
) {
    while (liquid_poll_event(event)) {
        switch (event->type) {
            case LIQUID_EVENT_QUIT:
                *isRunning = false;
                break;

            case LIQUID_EVENT_KEY_DOWN:
                char c = event->key.character;

                if (c >= 32 && c <= 126) { // 출력 가능한 ASCII 문자
                    if (inputLength < INPUT_BUFFER_SIZE - 1) {
                        inputBuffer[inputLength++] = c;
                        inputBuffer[inputLength] = '\0'; // null-terminate
                    }
                } else if (event->key.keycode == 22 || event->key.character == 8) { // Backspace
                    if (inputLength > 0) {
                        inputBuffer[--inputLength] = '\0';
                    }
                } else if (event->key.keycode == 36) { // Enter key
                    printf("입력된 문자열: %s\n", inputBuffer);
                    inputLength = 0;
                    inputBuffer[0] = '\0';
                }

                printf(" | Pressed Keycode =  %d\n | Ascii Letter = %c\n\n", event->key.keycode, event->key.character);
                if (event->key.keycode == 9) {
                    printf("\n\nEESC Pressed: Stop LiQuid!\n");
                    *isRunning = false;
                }
                    
                break;

            case LIQUID_EVENT_MOUSE_MOVE:
                *mouseX = event->mouse.x;
                *mouseY = event->mouse.y;
                break;

            case LIQUID_EVENT_MOUSE_DOWN:
                printf("Mouse down: button %d at (%d, %d)\n",
                    event->mouse.button, event->mouse.x, event->mouse.y);
                break;

            default: break;

        }
    }
}