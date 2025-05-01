#pragma once
#include <stdarg.h>
#include <string.h>
#include <canvas.h>
#include "Error.h"

#define LIQUID_RETURN_ERROR(code) \
    do { \
        LiquidResult _err = code; \
        if (_err != LIQUID_OK) { \
            liquidLogError("LiQuid Error: %s (%d)", liquidErrorToString(_err), _err); \
            canvasDebugPrint("Error: %s", liquidErrorToString(_err)); \
        } \
        return _err; \
    } while(0)


void LQDebugPrint(const char* fmt, ...);
void LQDebugFlush(Canvas* canvas);