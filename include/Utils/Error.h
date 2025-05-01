#pragma once 

typedef enum {

    LIQUID_OK = 0,
    LIQUID_ERROR_UNKNOWN = -1,
    LIQUID_ERROR_FILE_NOT_FOUND = -2,
    LIQUID_ERROR_IMAGE_LOAD_FAILED = -3,
    LIQUID_ERROR_FONT_LOAD_FAILED = -4,
    LIQUID_ERROR_INVALID_ARGUMENT = -5,

} LiQuidError;

const char* LQErrorToString(LiQuidError error);