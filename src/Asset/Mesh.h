#pragma once

#include <string>
#include <vector>
#include "Maths/Vec2.h"
#include "Maths/Vec3.h"

struct face {
    Maths::Vec3i v;
    Maths::Vec3i uv;
};

struct Mesh {
    std::vector<Maths::Vec3f> vertices;
    std::vector<Maths::Vec2f> uvs;
    std::vector<face>         faces;
    std::vector<Maths::Vec3f> normals;
};


Mesh LoadMesh(const char* path);
