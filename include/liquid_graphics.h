#pragma once
#include <stdint.h>
#include <stdlib.h>

//color chips
#define FAV_BLUE 0x00224488
#define FAV_GRAY 0x00334455

//저수준 함수
void liquidClear(uint32_t color);
void liquidDrawPixel(int x, int y, uint32_t color);
void liquidPresent(void);

//도형 그리기
void liquidDrawLine(int x0, int y0, int x1, int y1, uint32_t color);
void liquidDrawRect(int x, int y, int w, int h, uint32_t color);
void liquidFillRect(int x, int y, int w, int h, uint32_t color);
void liquidDrawCircle(int cx, int cy, int radius, uint32_t color);
void liquidFillCircle(int cx, int cy, int radius, uint32_t color);
