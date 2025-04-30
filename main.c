#include <liquid.h>
#include <Text/font.h>
#include <Text/liquid_text.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <platform/platform_switch.h>

char inputBuffer[INPUT_BUFFER_SIZE] = {0};      // 문자열 누적 버퍼 (256 MAX)
int inputLength = 0;                            // 현재 입력된 문자 수

int main(void) {
    liquid_init(800, 600, "Canvas test");

    bool running = true;
    int mouseX = 0;
    int mouseY = 0;

    while (running) {
        LiquidEvent event;
        while (liquid_poll_event(&event)) {
            switch (event.type) {
                case LIQUID_EVENT_QUIT:
                    running = false;
                    break;

                case LIQUID_EVENT_KEY_DOWN:
                    char c = event.key.character;

                    if (c >= 32 && c <= 126) { // 출력 가능한 ASCII 문자
                        if (inputLength < INPUT_BUFFER_SIZE - 1) {
                            inputBuffer[inputLength++] = c;
                            inputBuffer[inputLength] = '\0'; // null-terminate
                        }
                    } else if (event.key.keycode == 22 || event.key.character == 8) { // Backspace
                        if (inputLength > 0) {
                            inputBuffer[--inputLength] = '\0';
                        }
                    } else if (event.key.keycode == 36) { // Enter key
                        printf("입력된 문자열: %s\n", inputBuffer);
                        inputLength = 0;
                        inputBuffer[0] = '\0';
                    }

                    printf(" | Pressed Keycode =  %d\n | Ascii Letter = %c\n\n", event.key.keycode, event.key.character);
                    if (event.key.keycode == 9) 
                        running = false;
                    break;

                case LIQUID_EVENT_MOUSE_MOVE:
                    mouseX = event.mouse.x;
                    mouseY = event.mouse.y;
                    break;

                case LIQUID_EVENT_MOUSE_DOWN:
                    printf("Mouse down: button %d at (%d, %d)\n",
                        event.mouse.button, event.mouse.x, event.mouse.y);
                    break;

                default: break;

            }
        }

        Canvas* canvas = liquidBeginFrame();

        liquidClear(BG_COLOR);
        
        canvasSetStrokeColor(canvas, 0x00ffffff);
        canvasDrawLine(canvas, 100, 100, 700, 500);
        canvasDrawRect(canvas, 100, 100, 700, 500);

        canvasSetFillColor(canvas, FAV_GRAY);
        canvasFillCircle(canvas, mouseX, mouseY, 20);

        canvasScale(canvas, 2.0f, 2.0f);
        canvasPushTextScaled(canvas, 200, 100, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPp", 1);
        canvasPushTextScaled(canvas, 200, 200, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPp", 2);
        canvasPushTextScaled(canvas, 25, 300, "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPp", 3);

        canvasDrawCircle(canvas, 50, 200, 50);


        liquidEndFrame(canvas);
        liquidPresent();
    }

    liquid_shutdown();
    return 0;
}
