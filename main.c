#include <liquid.h>
#include <unistd.h>
#include <stdlib.h>

#include <platform/platform_switch.h>

int main(void) {
    liquid_init(800, 600, "Canvas test");

    for (int i = 0; i < 600; ++i) {
        liquidClear(0x00112233);

        Canvas* c = liquidBeginFrame();

        canvasSetStrokeColor(c, FAV_BLUE);
        canvasSetFillColor(c, 0x0000ff00);

        canvasFillRect(c, 100, 100, 200, 150);
        canvasDrawCircle(c, 100, 100, 80);

        liquidEndFrame(c);
        liquidPresent();
        usleep(16000);
    }

    liquid_shutdown();
    return 0;
}