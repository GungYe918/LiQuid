#pragma once

// 3x3 2D 변환 행렬
typedef struct {
    float m[3][3];
} Matrix2D;



Matrix2D matrixIdentity(void);
Matrix2D matrixTranslate(float tx, float ty);
Matrix2D matrixScale(float sx, float sy);
Matrix2D matrixRotate(float radians);
Matrix2D matrixMultiply(Matrix2D a, Matrix2D b);

void     matrixTransfromPoint(Matrix2D m, float *x, float *y);