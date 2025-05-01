#pragma once
#include <stdint.h>
#include <liquid_matrix.h>

typedef struct {
    Matrix2D transform;
    float strokeWidth;
    uint32_t strokeColor, fillColor;
    int fontSize;
} CanvasState;

typedef struct {
    CanvasState current;

    CanvasState stack[32];
    int stackTop;

    float scaleX, scaleY;
    int physicalWidth, physicalHeight;
} Canvas;

// Canvas 상태 관리
void canvasSave(Canvas* c);
void canvasRestore(Canvas* c);

// Canvas 이동 & 확대 & 회전
void canvasTranslate(Canvas*    c,  float tx, float ty);
void canvasScale(Canvas*        c,  float sx, float sy);
void canvasRotate(Canvas*       c,  float radians);


// Canvas 시작점 & 끝점 정의
Canvas* liquidBeginFrame(void);
void liquidEndFrame(Canvas* c);



void canvasSetStrokeColor(Canvas* c, uint32_t color);
void canvasSetStrokeWidth(Canvas* c, float width);

/*
void canvasSetFontSize(Canvas* c, int fontSize);
*/

void canvasDrawPixel(Canvas* c, int x, int y);
void canvasDrawLine(Canvas* c, int x0, int y0, int x1, int y1);
void canvasDrawRect(Canvas* c, int x, int y, int w, int h);

void canvasSetFillColor(Canvas* c, uint32_t color);
void canvasFillRect(Canvas* c, int x, int y, int w, int h);
void canvasDrawCircle(Canvas* c, int cx, int cy, int r);
void canvasFillCircle(Canvas* c, int cx, int cy, int r);

// 정다각형 정의
void canvasDrawRegularPolygon(Canvas* c, int cx, int cy, int r, int sides);
void canvasFillRegularPolygon(Canvas* c, int cx, int cy, int r, int sides);


// 임의의 점으로 다각형 생성
void canvasDrawPolygonFromPoints(Canvas* c, const float* points, int count);
void canvasFillPolygonFromPoints(Canvas* c, const float* points, int count);

// 클리핑 마스크 구현
void canvasClipRect(Canvas* c, int x, int y, int w, int h);
void canvasResetClip(Canvas* c);
