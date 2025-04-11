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

void liquidDrawLine(
    int x0, int y0, int x1, 
    int y1, uint32_t color
) {
    int dx = abs(x1 - x0); 
    int dy = abs(y1 - y0);

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        liquidDrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) 
            break;
        int e2 = 2*err;
        if (e2 > -dy) {  err -= dy;  x0 += sx;  }
        if (e2 <  dx) {  err += dx;  y0 += sy;  }
    }
}

//사각형 출력 함수
void liquidDrawRect(int x, int y, int w, int h, uint32_t color) {
    liquidDrawLine(x, y, x + w - 1, y, color);                  // top
    liquidDrawLine(x, y + h - 1, x + w - 1, y + h - 1, color);  // bottom
    liquidDrawLine(x, y, x, y + h - 1, color);                  // left
    liquidDrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);  // right
}


void liquidFillRect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < h; ++i) {
        liquidDrawLine(x, y + i, x + w - 1, y + i, color);
    }
}

//원 출력 함수
void liquidDrawCircle(int cx, int cy, int radius, uint32_t color) {
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



void liquidFillCircle(int cx, int cy, int radius, uint32_t color) {
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