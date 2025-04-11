#include <liquid.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
    liquid_init(800, 600, "Curve test");

    LiquidPath* path = liquidPathCreate();
    liquidPathMoveTo(path, 100, 300);
    liquidPathQuadraticTo(path, 400, 100, 700, 300);

    for (int i = 0; i < 600; ++i) {
        liquidClear(0x00112233);
        liquidPathStroke(path, 0x00ffffff);
        liquidPresent();
        usleep(16000);
    }

    liquidPathDestroy(path);

    liquid_shutdown();
    return 0;
}