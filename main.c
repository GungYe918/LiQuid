#include <liquid.h>
#include <unistd.h>
#include <stdlib.h>

#include <platform/platform_switch.h>

float star[] = {
    400, 200,
    420, 280,
    500, 280,
    440, 330,
    460, 400,
    400, 360,
    340, 400,
    360, 330,
    300, 280,
    380, 280
};

int main(void) {
    liquid_init(800, 600, "Canvas test");

    for (int i = 0; i < 160000; ++i) {
        liquidClear(0x00112233);

        Canvas* c = liquidBeginFrame();

        canvasSetStrokeWidth(c, 10.0f);

        canvasSetStrokeColor(c, FAV_BLUE);
        canvasSetFillColor(c, 0x0000ff00);

        canvasFillRect(c, 100, 100, 200, 150);
        canvasDrawCircle(c, 100, 100, 80);

        canvasSetStrokeColor(c, 0x00ff0000);
        canvasSetStrokeWidth(c, 8.0f);
        canvasDrawCircle(c, 500, 200, 100);
        canvasFillRegularPolygon(c, 500, 200, 100, 6);

        canvasSetFillColor(c, FAV_GRAY);
        canvasFillRegularPolygon(c, 600, 200, 100, 8);

        canvasSetStrokeColor(c, 0x00ffcc00);
        canvasSetStrokeWidth(c, 8.0f);

        canvasSetFillColor(c, FAV_BLUE);
        canvasFillPolygonFromPoints(c, star, 10);

        liquidEndFrame(c);
        liquidPresent();
        usleep(16000);
    }

    

    liquid_shutdown();
    return 0;
}