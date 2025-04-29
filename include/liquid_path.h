//liquid_path.h
#pragma once
#include <stdint.h>
#include <stdbool.h>

// 실행 환경별 분기
#ifdef LIQUID_LITE
#warning "You are using float-free liquid_roundf() instead of roundf() in math.h"
static inline int liquid_roundf(float x) {
    int i = (int)x;
    float frac = x - (float)i;

    if (x >= 0.0f) {
        return (frac >= 0.5f) ? i + 1 : i;
    } else {
        return (frac <= -0.5f) ? i - 1 : i;
    }
}

#else

#include <math.h>
static inline int liquid_roundf(float x) {
    return (int)roundf(x);
}

#endif

typedef enum {
    LIQUID_PATH_MOVE_TO,
    LIQUID_PATH_LINE_TO,
    LIQUID_PATH_QUADRATIC_TO,
    LIQUID_PATH_CLOSE
} LiquidPathCommand;

typedef struct {
    LiquidPathCommand command;
    float x, y;
    float cx, cy;
} LiquidPathElement;

typedef struct {
    LiquidPathElement* elements;
    int count;
    int capacity;
} LiquidPath;

typedef enum {
    LIQUID_FILL_RULE_EVEN_ODD,
    LIQUID_FILL_RULE_NON_ZERO      // 기본값
} LiquidFillRule;



LiquidPath* liquidPathCreate(void);
void liquidPathDestroy(LiquidPath* path);

void liquidPathMoveTo(LiquidPath* path, float x, float y);
void liquidPathLineTo(LiquidPath* path, float x, float y);
void liquidPathQuadraticTo(LiquidPath* path, float cx, float cy, float x, float y);
void liquidPathClose(LiquidPath* path);

void liquidPathStroke(LiquidPath* path, uint32_t color, float strokeWidth);


// Fill FUNC
void liquidPathFill(LiquidPath* path, uint32_t color);
