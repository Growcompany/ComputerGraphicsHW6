#pragma once

#include <cstdint>

// ─ Vec3, Vec4, Mat4 타입 선언
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };
struct Mat4 { float m[4][4]; };

// Vec3 연산
Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& v, float s);
float dot(const Vec3& a, const Vec3& b);
Vec3 cross(const Vec3& a, const Vec3& b);
Vec3 normalize(const Vec3& v);

// 행렬·벡터 유틸
Mat4 mul(const Mat4& A, const Mat4& B);
Vec4 mul(const Mat4& A, const Vec4& v);
Mat4 Identity();
Mat4 Translate(float tx, float ty, float tz);
Mat4 Scale(float sx, float sy, float sz);
Mat4 MakePerspective(float l, float r, float b, float t, float nz, float fz);
Mat4 MakeViewport(int nx, int ny);

// 보조 함수
int clampi(int v, int lo, int hi);
float min3(float a, float b, float c);
float max3(float a, float b, float c);
void barycentric(const Vec4& v0, const Vec4& v1, const Vec4& v2,
    float px, float py, float& w0, float& w1, float& w2);
