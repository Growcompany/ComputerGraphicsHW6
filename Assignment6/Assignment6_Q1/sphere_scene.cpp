#include "sphere_scene.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Definitions of globals
int   gNumVertices = 0;
int   gNumTriangles = 0;
Vec3* gVertexArray = nullptr;
int* gIndexBuffer = nullptr;

void create_scene() {
    int width = 32;
    int height = 16;

    gNumVertices = (height - 2) * width + 2;
    gNumTriangles = (height - 2) * (width - 1) * 2;

    gVertexArray = new Vec3[gNumVertices];
    gIndexBuffer = new int[3 * gNumTriangles];

    int t = 0;
    // middle latitude rings
    for (int j = 1; j < height - 1; ++j) {
        float theta = float(j) / (height - 1) * M_PI;
        for (int i = 0; i < width; ++i) {
            float phi = float(i) / (width - 1) * 2.0f * M_PI;
            gVertexArray[t].x = std::sinf(theta) * std::cosf(phi);
            gVertexArray[t].y = std::cosf(theta);
            gVertexArray[t].z = -std::sinf(theta) * std::sinf(phi);
            ++t;
        }
    }
    // north pole
    gVertexArray[t++] = { 0.0f, 1.0f, 0.0f };
    // south pole
    gVertexArray[t++] = { 0.0f, -1.0f, 0.0f };

    // build index buffer
    t = 0;
    for (int j = 0; j < height - 3; ++j) {
        int row0 = j * width;
        int row1 = (j + 1) * width;
        for (int i = 0; i < width - 1; ++i) {
            // first tri
            gIndexBuffer[t++] = row0 + i;
            gIndexBuffer[t++] = row1 + i + 1;
            gIndexBuffer[t++] = row0 + i + 1;
            // second tri
            gIndexBuffer[t++] = row0 + i;
            gIndexBuffer[t++] = row1 + i;
            gIndexBuffer[t++] = row1 + i + 1;
        }
    }
    // connect poles
    for (int i = 0; i < width - 1; ++i) {
        // north
        gIndexBuffer[t++] = (height - 2) * width;
        gIndexBuffer[t++] = i;
        gIndexBuffer[t++] = i + 1;
        // south
        gIndexBuffer[t++] = (height - 2) * width + 1;
        gIndexBuffer[t++] = (height - 3) * width + i + 1;
        gIndexBuffer[t++] = (height - 3) * width + i;
    }
}
