#pragma once
#include <stdint.h>
#include <stdlib.h>

//color chips
#define PALE_BLUE  0xFF224488
#define GRAY       0xFF334455
#define BG_COLOR   0xFF112233
#define WHITE      0xFFFFFFFF
#define BLACK      0xFF000000
#define RED        0xFFFF0000
#define GREEN      0xFF00FF00
#define BLUE       0xFF0000FF
#define YELLOW     0xFFFFFF00
#define CYAN       0xFF00FFFF
#define MAGENTA    0xFFFF00FF

// 추가 색상
#define ORANGE     0xFFFFA500
#define PURPLE     0xFF800080
#define BROWN      0xFFA52A2A
#define LIGHT_GRAY 0xFFD3D3D3
#define DARK_GRAY  0xFF2F4F4F
#define PINK       0xFFFFC0CB
#define NAVY       0xFF000080
#define TEAL       0xFF008080
#define OLIVE      0xFF808000

// 반투명 색상
#define SEMI_BLACK   0x88000000  // 50% 투명
#define SEMI_WHITE   0x88FFFFFF
#define TRANSPARENT  0x00000000  // 완전 투명

extern uint32_t clearColor;   // 초기 컬러값 검정으로 


//저수준 함수
void liquidClear(uint32_t color);
void liquidDrawPixel(int x, int y, uint32_t color);
void liquidPresent(void);

//도형 그리기
void liquidDrawLine(int x0, int y0, int x1, int y1, uint32_t color, float strokeWidth);