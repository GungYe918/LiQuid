#include <liquid_matrix.h>
#include <math.h>


// 단위 행렬 변환 - default
Matrix2D matrixIdentity(void) {
    return (Matrix2D){{
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    }};
}

// (tx, ty)만큼 평행이동 행렬
Matrix2D matrixTranslate(float tx, float ty) {
    Matrix2D m = matrixIdentity();

    m.m[0][2] = tx;
    m.m[1][2] = ty;

    // [1 0 tx]
    // [0 1 ty]
    // [0 0  1]

    return m;
}


// (sx, sy) 비율로 확대 & 축소 행렬
Matrix2D matrixScale(float sx, float sy) {
    Matrix2D m = matrixIdentity();

    m.m[0][0] = sx;
    m.m[1][1] = sy;

    // [sx  0  0]
    // [0  sy  0]
    // [0   0  1]

    // Ex: (sx=2, sy=2)     -> 2배 확대
    // Ex: (sx=-1, sy=-1)   -> y축 반전

    return m;
}


// radian값만큼 회전 행렬(반시계 방향) [회전 중심은 원점(0, 0)]
Matrix2D matrixRotate(float radians) {
    Matrix2D m = matrixIdentity();

    float c = cosf(radians);
    float s = sinf(radians);

    m.m[0][0] = c;      m.m[0][1] = -s;
    m.m[1][0] = s;      m.m[1][1] =  c;

    // [cos -sin  0]
    // [sin  cos  0]
    // [ 0    0   1]

    return m;
}

// 행렬(a, b)의 곱을 반환(a x b) but 교환법칙은 성립 X !!! 
Matrix2D matrixMultiply(Matrix2D a, Matrix2D b) {
    Matrix2D r;

    for (int y = 0; y < 3; ++y) 
        for (int x = 0; x < 3; ++x) {
            r.m[y][x] = 0;
            for (int k = 0; k < 3; ++k) 
                r.m[y][x] += a.m[y][k] * b.m[k][x];
        }

    return r;
}

//주어진 점(x, y)를 행렬 m을 통해 새로운 위치로 이동
void matrixTransfromPoint(Matrix2D m, float *x, float *y) {
    float tx = *x;
    float ty = *y;

    *x = m.m[0][0] * tx + m.m[0][1] * ty + m.m[0][2];
    *y = m.m[1][0] * tx + m.m[1][1] * ty + m.m[1][2];

    // [x']   [m00 m01 m02]   [x]
    // [y'] = [m10 m11 m12] × [y]
    //                        [1]
    //
    // z좌표는 1로 고정, 동차 좌표 변환
}