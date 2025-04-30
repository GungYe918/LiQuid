#pragma once
#include "stb_image.h"
#include <stdint.h>
#include <canvas.h>
#include <liquid_graphics.h>

typedef struct {
    int width;
    int height;
    int channels;
    uint8_t* data;
} Image;

Image* imageLoadFromFile(const char* filename);
void imageFree(Image* img);

void canvasPlaceImage(Canvas* c, int x, int y, const Image* img);