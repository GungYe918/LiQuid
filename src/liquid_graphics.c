#include "liquid_internal.h"
#include "../include/liquid_graphics.h"
#include "platform/platform.h"



void liquidClear(uint32_t color) {
    for (int i = 0; i < LIQUID_WIDTH * LIQUID_HEIGHT; ++i) {
        framebuffer[i] = color;
    }
}

void liquidDrawPixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= LIQUID_WIDTH || y < 0 || y >= LIQUID_HEIGHT) return;
    framebuffer[y * LIQUID_WIDTH + x] = color;
}

void liquidPresent(void) {
    platform_draw(framebuffer);
}

#include <math.h>

void liquidDrawLine(int x0, int y0, int x1, int y1, uint32_t color, float strokeWidth) {
    if (strokeWidth <= 1.0f) {
        // 얇은 선은 기본 브레젠험 알고리즘 사용
        int dx = abs(x1 - x0), dy = abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        while (true) {
            liquidDrawPixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx)  { err += dx; y0 += sy; }
        }
        return;
    }

    float dx = (float)(x1 - x0);
    float dy = (float)(y1 - y0);
    float length = sqrtf(dx * dx + dy * dy);
    if (length < 1e-6f) return;

    dx /= length;
    dy /= length;

    float nx = -dy;
    float ny = dx;

    float half = strokeWidth / 2.0f;

    // 픽셀 캐시 방지를 위해 offset을 0.5f 간격으로
    for (float offset = -half; offset <= half; offset += 0.5f) {
        float ox = nx * offset;
        float oy = ny * offset;

        float sx = x0 + ox;
        float sy = y0 + oy;
        float ex = x1 + ox;
        float ey = y1 + oy;

        // 부동소수 기반의 선 긋기
        float dx2 = fabsf(ex - sx);
        float dy2 = fabsf(ey - sy);
        int steps = (int)fmaxf(dx2, dy2);

        for (int i = 0; i <= steps; ++i) {
            float t = (float)i / steps;
            int px = (int)(sx + (ex - sx) * t + 0.5f);
            int py = (int)(sy + (ey - sy) * t + 0.5f);
            liquidDrawPixel(px, py, color);
        }
    }
}




//사각형 출력 함수
void liquidDrawRect(int x, int y, int w, int h, uint32_t color, float strokeWidth) {
    liquidDrawLine(x, y, x + w - 1, y, color, strokeWidth);                     // top
    liquidDrawLine(x, y + h - 1, x + w - 1, y + h - 1, color, strokeWidth);     // bottom
    liquidDrawLine(x, y, x, y + h - 1, color, strokeWidth);                    // left
    liquidDrawLine(x + w - 1, y, x + w - 1, y + h - 1, color, strokeWidth);     // right
}


void liquidFillRect(int x, int y, int w, int h, uint32_t color, float strokeWidth) {
    for (int i = 0; i < h; ++i) {
        liquidDrawLine(x, y + i, x + w - 1, y + i, color, strokeWidth);
    }
}

//원 출력 함수
void liquidDrawCircle(int cx, int cy, int radius, uint32_t color, float strokeWidth) {
    int x = radius;
    int y = 0; 
    int err = 0;

    while (x >= y) {
        liquidDrawPixel(cx + x, cy + y, color);
        liquidDrawPixel(cx + y, cy + x, color);
        liquidDrawPixel(cx - y, cy + x, color);
        liquidDrawPixel(cx - x, cy + y, color);
        liquidDrawPixel(cx - x, cy - y, color);
        liquidDrawPixel(cx - y, cy - x, color);
        liquidDrawPixel(cx + y, cy - x, color);
        liquidDrawPixel(cx + x, cy - y, color);

        y += 1;
        if (err <= 0) {

            err += 2*y + 1;

        } else {

            x   += -1;
            err += 2*(y - x) + 1;
        
        }
    }
}



void liquidFillCircle(int cx, int cy, int radius, uint32_t color, float strokeWidth) {
    int x = radius;
    int y = 0; 
    int err = 0;

    while (x >= y) {
        //수평선으로 채움
        for (int dx = -x; dx <= x; dx++) {
            liquidDrawPixel(cx + dx, cy + y, color);
            liquidDrawPixel(cx + dx, cy - y, color);
        }
        for (int dx = -y; dx <= y; dx++) {
            liquidDrawPixel(cx + dx, cy + x, color);
            liquidDrawPixel(cx + dx, cy - x, color);
        }

        y += 1;
        if (err <= 0) {

            err += 2*y + 1;

        } else {
            
            x += -1;
            err += 2*(y - x) + 1;

        }
    }
}