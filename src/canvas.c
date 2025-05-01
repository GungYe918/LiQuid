#include <canvas.h>
#include <stdlib.h>
#include <liquid_graphics.h>
#include <liquid_path.h>
#include <Utils/circle_table.h>


#define CIRCLE_SEGMENTS 64




// canvas 상태 관리 구현
void canvasSave(Canvas *c) {
    if (!c || c->stackTop >= 32) return;

    c->stack[c->stackTop++] = c->current;
}

void canvasRestore(Canvas* c) {
    if (!c || c->stackTop <= 0) return;

    c->current = c->stack[--(c->stackTop)];
}




void canvasTranslate(Canvas *c, float tx, float ty) {
    if (!c) return;

    Matrix2D t = matrixTranslate(tx, ty);
    c->current.transform = matrixMultiply(c->current.transform, t);
}

void canvasScale(Canvas *c, float sx, float sy) {  
    if (!c) return;

    Matrix2D s = matrixScale(sx, sy);
    c->current.transform = matrixMultiply(c->current.transform, s);

    
}

void canvasRotate(Canvas* c, float radians) {
    if (!c) return;

    Matrix2D r = matrixRotate(radians);
    c->current.transform = matrixMultiply(c->current.transform, r);
}



Canvas* liquidBeginFrame(void) {
    Canvas* c = malloc(sizeof(Canvas));
    c->current.strokeColor = 0xFFFFFFFF;    // 기본값 = 흰색
    c->current.fillColor = 0xFF000000;      // 기본값 = 검은색
    c->current.strokeWidth = 1.0f;
    c->current.fontSize = 18;               // 기본값 = 18

    c->current.transform = matrixIdentity();

    c->stackTop = 0;

    

    return c;
}

void liquidEndFrame(Canvas* c) {
    free(c);
}

void canvasSetStrokeColor(Canvas* c, uint32_t color) {
    if (c)
        c->current.strokeColor = color;
}

void canvasSetStrokeWidth(Canvas* c, float width) {
    if (!c || width <= 0.01f) return;
    c->current.strokeWidth = width;
}

/*
void canvasSetFontSize(Canvas* c, int fontSize) {
    if (!c || fontSize <= 1) return;
    c->current.fontSize = fontSize;
}
*/



void canvasDrawLine(Canvas* c, int x0, int y0, int x1, int y1) {
    if (!c) return;

    float fx0 = x0, fy0 = y0;
    float fx1 = x1, fy1 = y1;

    matrixTransfromPoint(c->current.transform, &fx0, &fy0);
    matrixTransfromPoint(c->current.transform, &fx1, &fy1);

    liquidDrawLine(
        (int)(fx0), (int)(fy0),
        (int)(fx1), (int)(fy1),
        c->current.strokeColor, c->current.strokeWidth
    );
}

void canvasDrawPixel(Canvas* c, int x, int y) {
    liquidDrawPixel(x, y, c->current.strokeColor);
}



// 도형 채우기
void canvasSetFillColor(Canvas* c, uint32_t color) {
    if (c) 
        c->current.fillColor = color;
}

// 직사각형 출력
void canvasDrawRect(Canvas* c, int x, int y, int w, int h) {
    if (!c) return;

    float fx0 = x,      fy0 = y;
    float fx1 = x + w,  fy1 = y;
    float fx2 = x + w,  fy2 = y + h;
    float fx3 = x,      fy3 = y + h;

    matrixTransfromPoint(c->current.transform, &fx0, &fy0);
    matrixTransfromPoint(c->current.transform, &fx1, &fy1);
    matrixTransfromPoint(c->current.transform, &fx2, &fy2);
    matrixTransfromPoint(c->current.transform, &fx3, &fy3);

    LiquidPath* path = liquidPathCreate();
    liquidPathMoveTo(path, (int)(fx0), (int)(fy0));
    liquidPathLineTo(path, (int)(fx1), (int)(fy1));
    liquidPathLineTo(path, (int)(fx2), (int)(fy2));
    liquidPathLineTo(path, (int)(fx3), (int)(fy3));
    liquidPathClose(path);

    //  (x, y) ───────────── (x + w, y)
    //      │                       │
    //      │                       │
    //      │                       │
    //  (x, y + h) ──────── (x + w, y + h)

    liquidPathStroke(path, c->current.strokeColor, c->current.strokeWidth);
    liquidPathDestroy(path);
}


// 채워진 직사각형 출력
void canvasFillRect(Canvas* c, int x, int y, int w, int h) {
    if (!c) return;

    float fx0 = x,      fy0 = y;
    float fx1 = x + w,  fy1 = y;
    float fx2 = x + w,  fy2 = y + h;
    float fx3 = x,      fy3 = y + h;

    matrixTransfromPoint(c->current.transform, &fx0, &fy0);
    matrixTransfromPoint(c->current.transform, &fx1, &fy1);
    matrixTransfromPoint(c->current.transform, &fx2, &fy2);
    matrixTransfromPoint(c->current.transform, &fx3, &fy3);

    LiquidPath* p = liquidPathCreate();
    liquidPathMoveTo(p, (int)(fx0), (int)(fy0));
    liquidPathLineTo(p, (int)(fx1), (int)(fy1));
    liquidPathLineTo(p, (int)(fx2), (int)(fy2));
    liquidPathLineTo(p, (int)(fx3), (int)(fy3));
    liquidPathClose(p);

    liquidPathFill(p, c->current.fillColor);
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
    liquidPathStroke(path, c->current.strokeColor, c->current.strokeWidth);
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
    liquidPathStroke(path, c->current.strokeColor, c->current.strokeWidth);
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
    liquidPathFill(path, c->current.fillColor);
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
    liquidPathStroke(path, c->current.strokeColor, c->current.strokeWidth);
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
    liquidPathFill(path, c->current.fillColor);
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
    liquidPathStroke(path, c->current.strokeColor, c->current.strokeWidth);
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
    liquidPathFill(path, c->current.fillColor);
    liquidPathDestroy(path);
}


/* TODO
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
*/