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

bool canvasLoadFont(Canvas *c, const char* path) {
    if (FT_Init_FreeType(&ftLib)) return false;
    if (FT_New_Face(ftLib, path, 0, &ftFace)) return false;

    FT_Set_Pixel_Sizes(ftFace, 0, c->current.fontSize);
    return true;
}

bool canvasSetFontSize(Canvas* c, int fontSize) {
    if (!c || !ftFace || fontSize < 1.0) return false;

    c->current.fontSize = fontSize;

    return FT_Set_Pixel_Sizes(ftFace, 0, fontSize) == 0;
}

void canvasPushText(Canvas *c, int x, int y, const char* text) {
    if (!c || !text) return;

    FT_Set_Pixel_Sizes(ftFace, 0, c->current.fontSize);
    int penX = x;

    while (*text) {
        char ch = *text++;

        if (FT_Load_Char(ftFace, ch, FT_LOAD_RENDER)) continue;

        FT_GlyphSlot g = ftFace->glyph;

        for (int row = 0; row < (g->bitmap.rows); ++row) {
            for (int column = 0; column < (g->bitmap.width); ++column) {
                int alpha = g->bitmap.buffer[row * g->bitmap.pitch + column];
                
                if (alpha > 0) {
                    //uint32_t color = c->current.strokeColor;
                    uint8_t r =     (c->current.strokeColor >> 16) & 0xFF;
                    uint8_t g_ =    (c->current.strokeColor >> 8) & 0xFF;
                    uint8_t b =     (c->current.strokeColor & 0xFF);

                    uint32_t blended = (alpha << 24) | (r << 16) | (g_ << 8) | b;
                    liquidDrawPixel(penX + g->bitmap_left + column, y - g->bitmap_top + row, blended);
                }
            }
        }
        
        penX += g->advance.x >> 6;
    }

}
#endif