#include <canvas.h>
#include <stdlib.h>
#include <liquid_graphics.h>
#include <liquid_path.h>
#include <utils/circle_table.h>

#define CIRCLE_SEGMENTS 64

struct CanvasState {
    float translateX, translateY;
    float strokeWidth;
    uint32_t strokeColor, fillColor;
};

struct Canvas {
    
    uint32_t strokeColor;
    uint32_t fillColor;

    bool hasClip;
    int clipX, clipY, clipW, clipH;

    float strokeWidth;
    

    CanvasState current;
    CanvasState stack[32];
    int stackTop;
};


Canvas* liquidBeginFrame(void) {
    Canvas* c = malloc(sizeof(Canvas));
    c->strokeColor = 0xFFFFFFFF;    // 기본값 = 흰색
    c->fillColor = 0xFF000000;      // 기본값 = 검은색
    c->strokeWidth = 1.0f;
    //c->current = c;
    

    return c;
}

void liquidEndFrame(Canvas* c) {
    free(c);
}

void canvasSetStrokeColor(Canvas* c, uint32_t color) {
    if (c)
        c->strokeColor = color;
}

void canvasSetStrokeWidth(Canvas* c, float width) {
    if (!c || width <= 0.01f) return;
    c->strokeWidth = width;
}

void canvasDrawLine(Canvas* c, int x0, int y0, int x1, int y1) {
    if (!c) {
        return;
    }

    liquidDrawLine(x0, y0, x1, y1, c->strokeColor, c->strokeWidth);
}




// 도형 채우기
void canvasSetFillColor(Canvas* c, uint32_t color) {
    if (c) 
        c->fillColor = color;
}

// 직사각형 출력
void canvasDrawRect(Canvas* c, int x, int y, int w, int h) {
    if (!c) {
        return;
    }

    LiquidPath* path = liquidPathCreate();
    liquidPathMoveTo(path, x, y);
    liquidPathLineTo(path, x + w, y);
    liquidPathLineTo(path, x + w, y + h);
    liquidPathLineTo(path, x, y + h);
    liquidPathClose(path);

    //  (x, y) ───────────── (x + w, y)
    //      │                       │
    //      │                       │
    //      │                       │
    //  (x, y + h) ──────── (x + w, y + h)

    liquidPathStroke(path, c->strokeColor, c->strokeWidth);
    liquidPathDestroy(path);
}

// 채워진 직사각형 출력
void canvasFillRect(Canvas* c, int x, int y, int w, int h) {
    if (!c) return;

    LiquidPath* p = liquidPathCreate();
    liquidPathMoveTo(p, x, y);
    liquidPathLineTo(p, x + w, y);
    liquidPathLineTo(p, x + w, y + h);
    liquidPathLineTo(p, x, y + h);
    liquidPathClose(p);

    liquidPathFill(p, c->fillColor);
    liquidPathDestroy(p);
}


#ifdef LIQUID_LITE
// 원 그리기 - LIQUID_LITE 버전
void canvasDrawCircle(Canvas* c, int cx, int cy, int r) {
    if (!c) return;

    LiquidPath* path = liquidPathCreate();
    int segments = (r > 100) ? 64 : (r > 30) ? 32 : 16;
    int step = CIRCLE_SEGMENTS / segments;

    for (int i = 0; i <= segments; ++i) {
        int idx = i * step;
        float x = cx + CIRCLE_COS_TABLE[idx] * r;
        float y = cy + CIRCLE_SIN_TABLE[idx] * r;
        if (i == 0)
            liquidPathMoveTo(path, x, y);
        else
            liquidPathLineTo(path, x, y);
    }

    liquidPathClose(path);
    liquidPathStroke(path, c->strokeColor, c->strokeWidth);
    liquidPathDestroy(path);
}

#else
// 원 그리기 - 일반 GPU환경용 버전
void canvasDrawCircle(Canvas* c, int cx, int cy, int r) {
    if (!c) return;

    LiquidPath* path = liquidPathCreate();
    int segments = 64;
    const float PI_MUL_2 = 6.2831853F;

    float angle = 0.0f;
    float angleStep = PI_MUL_2 / segments;
    for (int i = 0; i <= segments; ++i) {
        float x = cx + cosf(angle) * r;
        float y = cy + sinf(angle) * r;

        if (i == 0)
            liquidPathMoveTo(path, x, y);
        else
            liquidPathLineTo(path, x, y);

        angle += angleStep;
    }
    liquidPathClose(path);
    liquidPathStroke(path, c->strokeColor, c->strokeWidth);
    liquidPathDestroy(path);
}
#endif


void canvasFillCircle(Canvas* c, int cx, int cy, int r) {
    if (!c) return;

    LiquidPath* path = liquidPathCreate();
    int segments = 32;
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / segments * 2.0f * 3.14159265f;
        float x = cx + cosf(t) * r;
        float y = cy + sinf(t) * r;
        if (i == 0) liquidPathMoveTo(path, x, y);
        else        liquidPathLineTo(path, x, y);
    }
    liquidPathClose(path);
    liquidPathFill(path, c->fillColor);
    liquidPathDestroy(path);
}

void canvasDrawRegularPolygon(Canvas* c, int cx, int cy, int r, int sides) {
    if (!c || sides < 3) return;

    LiquidPath* path = liquidPathCreate();

    for (int i = 0; i <= sides; ++i) {
        float angle = (float)i / sides * 2.0f * 3.14159265f;
        float x = cx + cosf(angle) * r;
        float y = cy + sinf(angle) * r;

        if (i == 0) liquidPathMoveTo(path, x, y);
        else        liquidPathLineTo(path, x, y);
    }

    liquidPathClose(path);
    liquidPathStroke(path, c->strokeColor, c->strokeWidth);
    liquidPathDestroy(path);
}

void canvasFillRegularPolygon(Canvas* c, int cx, int cy, int r, int sides) {
    if (!c || sides < 3) return;

    LiquidPath* path = liquidPathCreate();

    for (int i = 0; i <= sides; ++i) {
        float angle = (float)i / sides * 2.0f * 3.14159265f;
        float x = cx + cosf(angle) * r;
        float y = cy + sinf(angle) * r;

        if (i == 0) liquidPathMoveTo(path, x, y);
        else        liquidPathLineTo(path, x, y);
    }

    liquidPathClose(path);
    liquidPathFill(path, c->fillColor);
    liquidPathDestroy(path);
}

void canvasDrawPolygonFromPoints(Canvas* c, const float* points, int count) {
    if (!c || !points || count < 3) return;

    LiquidPath* path = liquidPathCreate();

    for (int i = 0; i < count; ++i) {
        float x = points[i * 2 + 0];
        float y = points[i * 2 + 1];

        if (i == 0) liquidPathMoveTo(path, x, y);
        else        liquidPathLineTo(path, x, y);
    }

    liquidPathClose(path);
    liquidPathStroke(path, c->strokeColor, c->strokeWidth);
    liquidPathDestroy(path);
}

void canvasFillPolygonFromPoints(Canvas* c, const float* points, int count) {
    if (!c || !points || count < 3) return;

    LiquidPath* path = liquidPathCreate();

    for (int i = 0; i < count; ++i) {
        float x = points[i * 2];
        float y = points[i * 2 + 1];

        if (i == 0)     liquidPathMoveTo(path, x, y);
        else            liquidPathLineTo(path, x, y);
    }

    liquidPathClose(path);
    liquidPathFill(path, c->fillColor);
    liquidPathDestroy(path);
}



void canvasClipRect(Canvas* c, int x, int y, int w, int h) {
    if (!c) return;

    c->hasClip = true;
    c->clipX = x;
    c->clipY = y;
    c->clipW = w;
    c->clipH = h;
}

void canvasResetClip(Canvas* c) {
    if (!c) return;
    c->hasClip = false;
}