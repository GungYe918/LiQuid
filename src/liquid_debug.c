#include <Utils/Debug.h>
#include <Text/liquid_text.h>
#include <stdio.h>

#define MAX_DEBUG_LINES 20
static char debugLog[MAX_DEBUG_LINES][256];
static int debugLineCount = 0;

void LQDebugPrint(const char* fmt, ...) {
    if (debugLineCount >= MAX_DEBUG_LINES) {
        for (int i = 1; i < MAX_DEBUG_LINES; ++i)
            strcpy(debugLog[i - 1], debugLog[i]);
        debugLineCount = MAX_DEBUG_LINES - 1;
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(debugLog[debugLineCount], 128, fmt, args);
    va_end(args);

    debugLineCount++;
}

void LQDebugFlush(Canvas* canvas) {
    for (int i = 0; i < debugLineCount; ++i) {
        canvasPushText(canvas, 10, 10 + i * 12, debugLog[i]);
    }
}

void liquidLogError(const char* fmt, ...) {
    fprintf(stderr, "[LiQuid:ERROR] ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void liquidLogInfo(const char* fmt, ...) {
    fprintf(stdout, "[LiQuid:INFO] ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}