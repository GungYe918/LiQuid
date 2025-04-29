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

        Canvas* canvas = liquidBeginFrame();

        // 기본 설정
        canvasSetStrokeColor(canvas, 0x00FFFFFF);
        canvasSetFillColor(canvas, 0x0066CCFF);
        canvasSetStrokeWidth(canvas, 2.0f);

        // --- 기본 사각형 그리기 ---
        canvasSave(canvas); // 현재 변환 상태 저장
        canvasDrawRect(canvas, 100, 100, 200, 100);
        canvasRestore(canvas); // 복구

        // --- 회전 후 사각형 그리기 ---
        canvasSave(canvas); // 다시 저장

        // 회전 중심 좌표
        float centerX = 100 + 200 / 2.0f;
        float centerY = 100 + 100 / 2.0f;

        canvasTranslate(canvas, centerX, centerY);
        canvasRotate(canvas, 1.5708f); // 90도
        canvasTranslate(canvas, -centerX, -centerY);

        canvasDrawRect(canvas, 100, 100, 200, 100);

        canvasRestore(canvas); // 복구

        liquidEndFrame(canvas);
        liquidPresent();
        usleep(16000);
    }

    liquid_shutdown();
    return 0;
}
