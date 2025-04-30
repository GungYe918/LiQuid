#pragma once
#include <canvas.h>
#include <liquid_graphics.h>

#define INPUT_BUFFER_SIZE 256



#ifdef NO_FREETYPE
void canvasPushText(Canvas* c, int x, int y, const char* text);
void canvasPushTextScaled(Canvas* c, int x, int y, const char* text, int scale);
#else

#include <ft2build.h>
#include <stdbool.h>
#include <canvas.h>
#include FT_FREETYPE_H

bool canvasLoadFont(Canvas *c, const char* path);
void canvasPushText(Canvas* c, int x, int y, const char* text);
bool canvasSetFontSize(Canvas* c, int fontSize);
#endif