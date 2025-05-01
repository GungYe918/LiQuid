#pragma once
#include <stdarg.h>
#include <string.h>
#include <canvas.h>

void liquidDebugPrint(const char* fmt, ...);
void liquidDebugFlush(Canvas* canvas);