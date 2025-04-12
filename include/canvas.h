#pragma once
#include <stdint.h>

typedef struct Canvas Canvas;

Canvas* liquidBeginFrame(void);
void liquidEndFrame(Canvas* c);

void canvasSetStrokeColor(Canvas* c, uint32_t color);
void canvasDrawLine(Canvas* c, int x0, int y0, int x1, int y1);
void canvasDrawRect(Canvas* c, int x, int y, int w, int h);

void canvasSetFillColor(Canvas* c, uint32_t color);
void canvasFillRect(Canvas* c, int x, int y, int w, int h);
void canvasDrawCircle(Canvas* c, int cx, int cy, int r);