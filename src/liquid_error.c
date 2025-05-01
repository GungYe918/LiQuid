#include <Utils/Error.h>

const char* LQErrorToString(LiQuidError error) {
    switch (error) {
        case LIQUID_OK:
            return "Success!\n오류 없음";
        case LIQUID_ERROR_UNKNOWN:
            return "Unknown error occurred.\n알 수 없는 오류가 발생했습니다.\n[ErrorCode: 1 ]";
        
        // Core
        case LIQUID_ERROR_CORE_UNKNOWN:
            return "Core error occurred.\nLiQuid Core에서 알 수 없는 오류가 발생했습니다.\n[ErrorCode: 100 ]";
        case LIQUID_ERROR_NOT_INITIALIZED:
            return "LiQuid not initialized.\nLiQuid Core에서 초기화 오류가 발생했습니다.\n[ErrorCode: 101 ]";

        // Canvas
        case LIQUID_ERROR_CANVAS_NULL:
            return "Canvas is null.\nCanvas가 NULL입니다.\n[ErrorCode: 200 ]";
        case LIQUID_ERROR_CANVAS_INVALID_ARG:
            return "Invalid argument.\nCanvas에 잘못된 인자가 전달되었습니다.\n[ErrorCode: 201 ]";
        case LIQUID_ERROR_CANVAS_OUT_OF_MEMORY:
            return "Out of memory.\nCanvas에서 메모리 부족 오류가 발생했습니다.\n[ErrorCode: 202 ]";

        // Image
        case LIQUID_ERROR_IMAGE_NOT_FOUND:
            return "Image not found.\n이미지를 찾을 수 없습니다.\n[ErrorCode: 300 ]";
        case LIQUID_ERROR_IMAGE_LOAD_FAILED:
            return "Image load failed.\n이미지 로드에 실패했습니다.\n[ErrorCode: 301 ]";
        case LIQUID_ERROR_IMAGE_UNSUPPORTED_FORMAT:
            return "Unsupported image format.\n지원하지 않는 이미지 포멧입니다.\n[ErrorCode: 302 ]";

        // Font
        case LIQUID_ERROR_FONT_NOT_FOUND:
            return "Font not found.\n폰트를 찾을 수 없습니다.\n[ErrorCode: 400 ]";
        case LIQUID_ERROR_FONT_UNSUPPORTED_FORMAT:
            return "Unsupported font format.\n지원하지 않는 폰트 포멧입니다.\n[ErrorCode: 401 ]";
        case LIQUID_ERROR_FONT_LOAD_FAILED:
            return "Font load failed.\n폰트 로드에 실패했습니다.\n[ErrorCode: 402 ]";
        
            // File
        case LIQUID_ERROR_FILE_NOT_FOUND:
            return "File not found.\n파일을 찾을 수 없습니다.\n[ErrorCode: 500 ]";
        case LIQUID_ERROR_UNSUPPORTED_FILE_FORMAT:
            return "Unsupported file format.\n지원하지 않는 파일 포멧입니다.\n[ErrorCode: 501 ]";
        case LIQUID_ERROR_INVALID_ASSET_URL:
            return "Invalid asset URL.\n잘못된 Asset URL입니다.\n[ErrorCode: 502 ]";

        // Event System
        case LIQUID_ERROR_EVENT_SYSTEM:
            return "Event system error.\n이벤트 시스템에서 오류가 발생했습니다.\n[ErrorCode: 600 ]";

        // User Error
        case LIQUID_ERROR_INVALID_ARGUMENT:
            return "Invalid argument.\n잘못된 인자입니다.\n[ErrorCode: 700 ]";
        case LIQUID_ERROR_OUT_OF_MEMORY:
            return "Out of memory.\n메모리 부족 오류가 발생했습니다.\n[ErrorCode: 701 ]";
        case LIQUID_ERROR_INVALID_OPERATION:
            return "Invalid operation.\n잘못된 작업 혹은 허용되지 않는 작업이 발생했습니다..\n[ErrorCode: 702 ]";
        default:
            return "ERROR!";
    }

}