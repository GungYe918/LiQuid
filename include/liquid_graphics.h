#pragma once
#include <stdint.h>
#include <stdlib.h>

//color chips
#define FAV_BLUE 0x00224488
#define FAV_GRAY 0x00334455
#define BG_COLOR 0x00112233

//저수준 함수
void liquidClear(uint32_t color);
void liquidDrawPixel(int x, int y, uint32_t color);
void liquidPresent(void);

//도형 그리기
void liquidDrawLine(int x0, int y0, int x1, int y1, uint32_t color, float strokeWidth);