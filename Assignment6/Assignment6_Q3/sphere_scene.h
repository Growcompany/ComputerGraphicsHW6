#pragma once
#include <cstddef>
#include "graphics_math.h"

// Mesh globals declared in implementation
extern int   gNumVertices;
extern int   gNumTriangles;
extern Vec3* gVertexArray;
extern int* gIndexBuffer;

// Generate unit-sphere mesh
void create_scene();