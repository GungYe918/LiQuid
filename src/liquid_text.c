#include <Text/liquid_text.h>
#include <Text/font.h>

#ifdef NO_FREETYPE
void canvasPushText(Canvas* c, int x, int y, const char* text) {
    if (!c || !text) return;

    int cursorX = x;
    while (*text) {
        unsigned char ch = (unsigned char)*text++;

        const uint8_t* glyph = font8x8_basic[ch];
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                if ((glyph[row] >> col) & 1) {
                    canvasDrawPixel(c, cursorX + col, y + row);
                }
            }
        }

        cursorX += 8;
    }
}


void canvasPushTextScaled(Canvas* c, int x, int y, const char* text, int scale) {
    if (!c || !text || scale < 1) return;
    if (scale == 1) {
        canvasPushText(c, x, y, text);
        return;
    }

    int cursorX = x;
    while (*text) {
        unsigned char ch = (unsigned char)*text++;

        const uint8_t* glyph = font8x8_basic[ch];
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                if ((glyph[row] >> col) & 1) {
                    for (int dy = 0; dy < scale; ++dy) {
                        for (int dx = 0; dx < scale; ++dx) {
                            canvasDrawPixel(c, cursorX + col * scale + dx, y + row * scale + dy);
                        }
                    }
                }
            }
        }

        cursorX += 8 * scale;
    }
}
#else
#include FT_FREETYPE_H

static FT_Library ftLib;
static FT_Face ftFace;

bool canvasLoadFont(const char* path, int pixelSize) {
    return true;
}
#endif