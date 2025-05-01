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

#define TAB_SPACES 4
#define LINEBUF_SIZE 1024

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

uint32_t utf8_next_char(const char** p) {
    const unsigned char* s = (const unsigned char*)*p;
    uint32_t cp;

    // 첫 바이트 기준으로 UTF-8 시퀀스 길이 계산
    unsigned char byte1 = s[0];
    int len = 
        (byte1 < 0x80) ? 1 :
        (byte1 < 0xE0) ? 2 :
        (byte1 < 0xF0) ? 3 :
        (byte1 < 0xF8) ? 4 : 1;  // fallback

    // 미리 32비트로 복사해두고 필요한 비트만 추출
    cp = 0;

    switch (len) {
        case 1:
            cp = byte1;
            break;

        case 2:
            cp = ((byte1 & 0x1F) << 6) |
                 (s[1] & 0x3F);
            break;

        case 3:
            cp = ((byte1 & 0x0F) << 12) |
                 ((s[1] & 0x3F) << 6) |
                 (s[2] & 0x3F);
            break;

        case 4:
            cp = ((byte1 & 0x07) << 18) |
                 ((s[1] & 0x3F) << 12) |
                 ((s[2] & 0x3F) << 6) |
                 (s[3] & 0x3F);
            break;
        
        default:
            cp = '?';
            len = 1;
            break;
    }

    *p += len;
    return cp;
}

// FreeType 기반 UTF-8 문자열의 너비 측정
int canvasMeasureTextWidth(Canvas* c, const char* utf8_text) {
    if (!c || !utf8_text || !ftFace) return -1;

    FT_Set_Pixel_Sizes(ftFace, 0, c->current.fontSize);

    int width = 0;
    const char* p = utf8_text;

    while (*p) {
        uint32_t cp = utf8_next_char(&p);

        if (cp == '\n') break;
        if (cp == '\t') {
            for (int i = 0; i < TAB_SPACES; ++i) {
                if (FT_Load_Char(ftFace, ' ', FT_LOAD_DEFAULT) == 0)
                    width += ftFace->glyph->advance.x >> 6;
            }
            continue;
        }

        if (FT_Load_Char(ftFace, cp, FT_LOAD_DEFAULT) == 0)
            width += ftFace->glyph->advance.x >> 6;
    }

    return width;
}



// 텍스트 렌더링
void canvasPushText(Canvas *c, int x, int y, const char* text) {
    if (!c || !text) return;

    FT_Set_Pixel_Sizes(ftFace, 0, c->current.fontSize);
    int penX = x;

    const char* p = text;

    while (*p) {
        uint32_t cp = utf8_next_char(&p);

        if (FT_Load_Char(ftFace, cp, FT_LOAD_RENDER)) continue;

        FT_GlyphSlot g = ftFace->glyph;

        for (int row = 0; row < g->bitmap.rows; ++row) {
            for (int col = 0; col < g->bitmap.width; ++col) {
                int alpha = g->bitmap.buffer[row * g->bitmap.pitch + col];
                if (alpha > 0) {
                    uint8_t r = (c->current.strokeColor >> 16) & 0xFF;
                    uint8_t g_ = (c->current.strokeColor >> 8) & 0xFF;
                    uint8_t b = c->current.strokeColor & 0xFF;
                    uint32_t blended = (alpha << 24) | (r << 16) | (g_ << 8) | b;
                    liquidDrawPixel(penX + g->bitmap_left + col, y - g->bitmap_top + row, blended);
                }
            }
        }

        penX += g->advance.x >> 6;
    }
}



// 줄바꿈 포함 텍스트 출력
LiQuidError canvasPlaceTextField(Canvas *c, int x, int y, int width, const char* text) {
    if (!c || !text) return LIQUID_ERROR_CANVAS_NULL;

    int cursorX = x;
    int cursorY = y;

    int fontSize = c->current.fontSize;
    int lineHeight = (int)(fontSize * 1.2f);

    const char* p = text;

    char lineBuf[LINEBUF_SIZE];
    int bufIndex = 0;
    int lineWidth = 0;

    FT_Set_Pixel_Sizes(ftFace, 0, fontSize);

    while (*p) {
        const char* charStart = p;
        uint32_t cp = utf8_next_char(&p);

        if (cp == '\n') {
            lineBuf[bufIndex] = '\0';
            canvasPushText(c, cursorX, cursorY, lineBuf);
            cursorY += lineHeight;
            bufIndex = 0;
            lineWidth = 0;
            continue;
        }

        if (cp == '\t') {
            int tabWidth = 0;
            for (int i = 0; i < TAB_SPACES; ++i) {
                if (FT_Load_Char(ftFace, ' ', FT_LOAD_DEFAULT) == 0)
                    tabWidth += ftFace->glyph->advance.x >> 6;
            }

            if (lineWidth + tabWidth > width) {
                lineBuf[bufIndex] = '\0';
                canvasPushText(c, cursorX, cursorY, lineBuf);
                cursorY += lineHeight;
                bufIndex = 0;
                lineWidth = 0;
            } else {
                for (int i = 0; i < TAB_SPACES && bufIndex < LINEBUF_SIZE - 1; ++i)
                    lineBuf[bufIndex++] = ' ';
                lineWidth += tabWidth;
            }
            continue;
        }

        // 일반 문자 너비
        if (FT_Load_Char(ftFace, cp, FT_LOAD_DEFAULT) != 0) continue;
        int glyphWidth = ftFace->glyph->advance.x >> 6;

        if (lineWidth + glyphWidth > width) {
            lineBuf[bufIndex] = '\0';
            canvasPushText(c, cursorX, cursorY, lineBuf);
            cursorY += lineHeight;
            bufIndex = 0;
            lineWidth = 0;
        }

        int charLen = p - charStart;
        if (bufIndex + charLen < LINEBUF_SIZE - 1) {
            memcpy(&lineBuf[bufIndex], charStart, charLen);
            bufIndex += charLen;
            lineWidth += glyphWidth;
        } else {
            return LIQUID_ERROR_OUT_OF_MEMORY;
        }
    }

    if (bufIndex > 0) {
        lineBuf[bufIndex] = '\0';
        canvasPushText(c, cursorX, cursorY, lineBuf);
    }

    return LIQUID_OK;
}

#endif