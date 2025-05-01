#include <Image/liquid_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>



int hasValidImageExtension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;

    char ext[8] = {0};
    size_t len = strlen(dot + 1);
    if (len >= sizeof(ext)) return 0;

    for (size_t i = 0; i < len; i++) {
        ext[i] = (char)tolower(dot[1 + i]);
    }

    return strcmp(ext, "png")   == 0 ||
           strcmp(ext, "jpg")   == 0 ||
           strcmp(ext, "jpeg")  == 0 ||
           strcmp(ext, "bmp")   == 0 ;
}

LiQuidError imageLoadFromFile(const char* filename, Image** outImage) {
    if (!filename || !outImage || strlen(filename)==0) return LIQUID_ERROR_INVALID_ARGUMENT;

    
    Image* img = malloc(sizeof(Image));
    if (!img) {
        return LIQUID_ERROR_UNKNOWN;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        return LIQUID_ERROR_INVALID_ASSET_URL;
    }

    if (!hasValidImageExtension(filename)) return LIQUID_ERROR_IMAGE_UNSUPPORTED_FORMAT;


    img->data = stbi_load(filename, &img->width, &img->height, &img->channels, 4);
    img->channels = 4;

    if (!img->data) {
        free(img);
        return LIQUID_ERROR_IMAGE_LOAD_FAILED;
    }

    *outImage = img;
    return LIQUID_OK;
}

LiQuidError imageFree(Image* img) {
    if (img) {
        stbi_image_free(img->data);
        free(img);
        return LIQUID_OK;
    } else {
        return LIQUID_ERROR_OUT_OF_MEMORY;
    }
}

LiQuidError canvasPlaceImage(Canvas* c, int x, int y, const Image* img) {
    if (!c || !img || !img->data) {
        return LIQUID_ERROR_IMAGE_LOAD_FAILED;
    }

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
    
    return LIQUID_OK;
}
