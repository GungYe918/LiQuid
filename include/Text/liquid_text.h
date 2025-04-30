#pragma once
#include <canvas.h>
#define INPUT_BUFFER_SIZE 256



#ifdef NO_FREETYPE
void canvasPushText(Canvas* c, int x, int y, const char* text);
void canvasPushTextScaled(Canvas* c, int x, int y, const char* text, int scale);
#else

#include <ft2build.h>
#include <stdbool.h>
#include FT_FREETYPE_H

static FT_Library ftLib;
static FT_Face ftFace;

bool canvasLoadFont(const char* path, int pixelSize);
#endif