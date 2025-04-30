#include <Image/liquid_image.h>
#include <stdlib.h>

Image* imageLoadFromFile(const char* filename) {
    Image* img = malloc(sizeof(Image));
    if (!img) return NULL;

    img->data = stbi_load(filename, &img->width, &img->height, &img->channels, 4);
    img->channels = 4;

    if (!img->data) {
        free(img);
        return NULL;
    }

    return img;
}

void imageFree(Image* img) {
    if (img) {
        stbi_image_free(img->data);
        free(img);
    }
}

void canvasPlaceImage(Canvas* c, int x, int y, const Image* img) {
    if (!c || !img || !img->data) return;

    for (int row = 0; row < img->height; ++row) {
        for (int column = 0; column < img->width; ++column) {
            int index = (row * img->width + column) * 4;
            uint8_t r = img->data[index + 0];
            uint8_t g = img->data[index + 1];
            uint8_t b = img->data[index + 2];
            uint8_t a = img->data[index + 3];

            uint32_t color  = (a << 24) | (r << 16) | (g << 8) | b;
            liquidDrawPixel(x + column, y + row, color);
        }
    }
}