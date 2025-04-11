#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <liquid_path.h>
#include <liquid_graphics.h> // -> LiquidDrawPixel

static void pathEnsureCapacity(LiquidPath* path) {
    if (path->count >= path->capacity) {
        path->capacity = path->capacity ? path->capacity*2 : 16;
        path->elements = realloc(path->elements, path->capacity * sizeof(LiquidPathElement));
    }
}

LiquidPath* liquidPathCreate(void) {
    LiquidPath* path = malloc(sizeof(LiquidPath));
    path->count = 0;
    path->capacity = 0;
    path->elements = NULL;
    
    return path;
}

void liquidPathDestroy(LiquidPath* path) {
    if (!path) {
        return;
    }

    free(path->elements);
    free(path);
}

void liquidPathMoveTo(LiquidPath* path, float x, float y) {
    pathEnsureCapacity(path);
    path->elements[path->count++] = 
        (LiquidPathElement) {  LIQUID_PATH_MOVE_TO, x, y  };
}

void liquidPathLineTo(LiquidPath* path, float x, float y) {
    pathEnsureCapacity(path);
    path->elements[path->count++] = 
        (LiquidPathElement) {  LIQUID_PATH_LINE_TO, x, y  };
}

void liquidPathQuadraticTo(LiquidPath* path, float cx, float cy, float x, float y) {
    pathEnsureCapacity(path);
    LiquidPathElement e = {
        .command = LIQUID_PATH_QUADRATIC_TO,
        .x = x,
        .y = y,
        .cx = cx,
        .cy = cy
    };
    path->elements[path->count++] = e;
}


void liquidPathClose(LiquidPath* path) {
    pathEnsureCapacity(path);
    path->elements[path->count++] = 
        (LiquidPathElement) {   LIQUID_PATH_CLOSE, 0, 0   };
}

void liquidPathStroke(LiquidPath* path, uint32_t color) {
    float sx = 0;   // 시작점 (x)
    float sy = 0;   // 시작점 (y)

    float lx = 0;   // 종점 (x)
    float ly = 0;   // 종점 (y)

    for (int i = 0; i < path->count; ++i) {
        LiquidPathElement e = path->elements[i];

        switch (e.command) {
            case LIQUID_PATH_MOVE_TO:
                lx = e.x;
                ly = e.y;
                sx = e.x;
                sy = e.y;
                break;
            
            case LIQUID_PATH_LINE_TO:
                // 선 그리기 (FPU + 정수 변환) -> 나중에 최적화
                liquidDrawLine(
                    liquid_roundf(lx),  liquid_roundf(ly),
                    liquid_roundf(e.x), liquid_roundf(e.y), 
                    color
                );
                lx = e.x;
                ly = e.y;
                break;
            
            case LIQUID_PATH_CLOSE:
                // 종점 -> 시점 선
                liquidDrawLine(
                    liquid_roundf(lx), liquid_roundf(ly),
                    liquid_roundf(sx), liquid_roundf(sy),
                    color
                );
                lx = sx;
                ly = sy;
                break;
            
            case LIQUID_PATH_QUADRATIC_TO:
                float cx1 = e.cx, cy1 = e.cy;
                float x1 = e.x,  y1 = e.y;

                const int segments = 20;
                float prevX = lx, prevY = ly;

                for (int j = 1; j <= segments; ++j) {
                    float t = (float)j / segments;
                    float u = 1 - t;
                    float x = u * u * lx + 2 * u * t * cx1 + t * t * x1;
                    float y = u * u * ly + 2 * u * t * cy1 + t * t * y1;
                    liquidDrawLine((int)roundf(prevX), (int)roundf(prevY),
                                (int)roundf(x),     (int)roundf(y), color);
                    prevX = x;
                    prevY = y;
                }

                lx = x1;
                ly = y1;
                break;
        }
    }
}

static void fillFlatPolygon(LiquidPath* path, uint32_t color) {
    #define MAX_VERTICES 256    // 256개 까지만 처리 (임시)
    int count = 0;
    int vx[MAX_VERTICES], vy[MAX_VERTICES];

    float startX = 0, startY = 0;
    float lastX = 0, lastY = 0;
    bool isStarted = false;

    for (int i = 0; i < path->count; ++i) {
        LiquidPathElement e = path->elements[i];

        if (e.command == LIQUID_PATH_MOVE_TO) {
            if (isStarted) {
                vx[count] = liquid_roundf(startX);
                vy[count] = liquid_roundf(startY);
                count++;
            }

            startX = lastX = e.x;
            startY = lastY = e.y;
            vx[0] = liquid_roundf(e.x);
            vy[0] = liquid_roundf(e.y);
            count = 1;
            isStarted = true;
        }
        else if (e.command == LIQUID_PATH_LINE_TO) {
            if (count < MAX_VERTICES) {
                vx[count] = liquid_roundf(e.x);
                vy[count] = liquid_roundf(e.y);
                count++;
                lastX = e.x;
                lastY = e.y;
            }
        }
        else if (e.command == LIQUID_PATH_CLOSE) {
            if (count < MAX_VERTICES) {
                vx[count] = liquid_roundf(startX);
                vy[count] = liquid_roundf(startY);
                count++;
            }
        }
    }

    // scanline 알고리즘
    if (count < 3) return;

    int minY = vy[0], maxY = vy[0];
    for (int i = 1; i < count; ++i) {
        if (vy[i] < minY) minY = vy[i];
        if (vy[i] > maxY) maxY = vy[i];
    }

    for (int y = minY; y <= maxY; ++y) {
        int nodes[MAX_VERTICES];
        int nodeCount = 0;

        int j = count - 1;
        for (int i = 0; i < count; ++i) {
            if ((vy[i] < y && vy[j] >= y) || (vy[j] < y && vy[i] >= y)) {
                int x = vx[i] + (y - vy[i]) * (vx[j] - vx[i]) / (vy[j] - vy[i]);
                nodes[nodeCount++] = x;
            }
            j = i;
        }

    // 정렬
    for (int i = 0; i < nodeCount - 1; ++i) {
        for (int j = i + 1; j < nodeCount; ++j) {
            if (nodes[i] > nodes[j]) {
                int tmp = nodes[i]; nodes[i] = nodes[j]; nodes[j] = tmp;
            }
        }
    }

    for (int i = 0; i < nodeCount; i += 2) {
        if (i + 1 >= nodeCount) break;
        for (int x = nodes[i]; x <= nodes[i + 1]; ++x) {
            liquidDrawPixel(x, y, color);
        }
    }
}

}

void liquidPathFill(LiquidPath* path, uint32_t color) {
    fillFlatPolygon(path, color);
}