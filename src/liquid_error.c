#include <Utils/Error.h>

const char* LQErrorToString(LiQuidError error) {
    switch (error) {
        case LIQUID_OK:
            return "Success!";
        case LIQUID_ERROR_UNKNOWN:
            return "Unknown error";
        case LIQUID_ERROR_FILE_NOT_FOUND:
            return "File not found";
        case LIQUID_ERROR_IMAGE_LOAD_FAILED:
            return "Image load failed";
        case LIQUID_ERROR_FONT_LOAD_FAILED:
            return "Font load failed";
        case LIQUID_ERROR_INVALID_ARGUMENT:
            return "Invalid argument";
        default:
            return "Unrecognized error code";
    }
}