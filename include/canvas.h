#pragma once
#include <stdint.h>

typedef struct Canvas Canvas;
typedef struct CanvasState CanvasState;

Canvas* liquidBeginFrame(void);
void liquidEndFrame(Canvas* c);

void canvasSetStrokeColor(Canvas* c, uint32_t color);
void canvasSetStrokeWidth(Canvas* c, float    width);

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
