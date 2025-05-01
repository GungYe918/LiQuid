#pragma once
#include "stb_image.h"
#include <stdint.h>
#include <canvas.h>
#include <liquid_graphics.h>
#include <Utils/Debug.h>

typedef struct {
    int width;
    int height;
    int channels;
    uint8_t* data;
} Image;

LiQuidError imageLoadFromFile(const char* filename, Image** outImage);
LiQuidError imageFree(Image* img);

LiQuidError canvasPlaceImage(Canvas* c, int x, int y, const Image* img);