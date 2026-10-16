#pragma once

#include <vector>

#include "Maths/Vec3.h"
#include "Maths/Vec2.h"
#include "Platform/MemoryMappedFile.h"

struct face {
	Maths::Vec3i v;
	Maths::Vec3i uv;
};

bool LoadObj(const char* path, std::vector<Maths::Vec3f>& outVertices, std::vector<Maths::Vec2f>& outuvs, std::vector<face>& outFaces);







