#pragma once
//#include <platform/platform.h>

// x11 환경
#if defined(LIQUID_BACKEND_X11)
#include "platform_x11.c"

// xrender 환경
#elif defined(LIQUID_BACKEND_XRENDER)
#include "platform_xrender.c"

// wayland 환경
#elif defined(LIQUID_BACKEND_WAYLAND)
#include "platform_wayland.c"

// win32 환경
#elif defined(LIQUID_BACKEND_WIN32)
#include "platform_win32.c"

// 프레임 버퍼 환경
#elif defined(LIQUID_BACKEND_FB)
#include "platform_fb.c"

#else
//#error "No Platform backend selected"
#endif

