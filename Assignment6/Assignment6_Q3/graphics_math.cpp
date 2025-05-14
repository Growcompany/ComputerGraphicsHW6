#include "graphics_math.h"
#include <cmath>
#include <algorithm>

// Vec3 연산 정의
Vec3 operator+(const Vec3& a, const Vec3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}
Vec3 operator-(const Vec3& a, const Vec3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
Vec3 operator*(const Vec3& v, float s) {
    return { v.x * s, v.y * s, v.z * s };
}
float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
    };
}
Vec3 normalize(const Vec3& v) {
    float L = std::sqrt(dot(v, v));
    return { v.x / L, v.y / L, v.z / L };
}

// 행렬·벡터 유틸
Mat4 mul(const Mat4& A, const Mat4& B) {
    Mat4 R{};
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) {
        float s = 0;
        for (int k = 0; k < 4; ++k) s += A.m[i][k] * B.m[k][j];
        R.m[i][j] = s;
    }
    return R;
}
Vec4 mul(const Mat4& A, const Vec4& v) {
    return {
      A.m[0][0] * v.x + A.m[0][1] * v.y + A.m[0][2] * v.z + A.m[0][3] * v.w,
      A.m[1][0] * v.x + A.m[1][1] * v.y + A.m[1][2] * v.z + A.m[1][3] * v.w,
      A.m[2][0] * v.x + A.m[2][1] * v.y + A.m[2][2] * v.z + A.m[2][3] * v.w,
      A.m[3][0] * v.x + A.m[3][1] * v.y + A.m[3][2] * v.z + A.m[3][3] * v.w
    };
}
Mat4 Identity() {
    Mat4 I{};
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) I.m[i][j] = (i == j ? 1.0f : 0.0f);
    return I;
}
Mat4 Translate(float tx, float ty, float tz) {
    Mat4 M = Identity(); M.m[0][3] = tx; M.m[1][3] = ty; M.m[2][3] = tz; return M;
}
Mat4 Scale(float sx, float sy, float sz) {
    Mat4 M = Identity(); M.m[0][0] = sx; M.m[1][1] = sy; M.m[2][2] = sz; return M;
}
Mat4 MakePerspective(float l, float r, float b, float t, float n, float f) {
    Mat4 P{};
    P.m[0][0] = 2 * n / (r - l);   P.m[0][2] = (r + l) / (r - l);
    P.m[1][1] = 2 * n / (t - b);   P.m[1][2] = (t + b) / (t - b);
    P.m[2][2] = -(f + n) / (f - n); P.m[2][3] = -2 * f * n / (f - n);
    P.m[3][2] = -1;
    return P;
}

Mat4 MakeViewport(int nx, int ny) {
    Mat4 W = Identity();
    W.m[0][0] = nx / 2.0f;
    W.m[0][3] = (nx - 1) / 2.0f;
    W.m[1][1] = -ny / 2.0f;
    W.m[1][3] = (ny - 1) / 2.0f;
    W.m[2][2] = 0.5f;
    W.m[2][3] = 0.5f;
    return W;
}

int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
float min3(float a, float b, float c) { return std::min(a, std::min(b, c)); }
float max3(float a, float b, float c) { return std::max(a, std::max(b, c)); }

void barycentric(const Vec4& v0, const Vec4& v1, const Vec4& v2,
    float px, float py,
    float& w0, float& w1, float& w2) {
    float x0 = v0.x, y0 = v0.y, x1 = v1.x, y1 = v1.y, x2 = v2.x, y2 = v2.y;
    float d = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);
    w0 = ((y1 - y2) * (px - x2) + (x2 - x1) * (py - y2)) / d;
    w1 = ((y2 - y0) * (px - x2) + (x0 - x2) * (py - y2)) / d;
    w2 = 1.0f - w0 - w1;
}