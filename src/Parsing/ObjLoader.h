#pragma once

#include <vector>

#include "Maths/Vec3.h"
#include "Platform/MemoryMappedFile.h"


bool LoadObj(const char* path, std::vector<Maths::Vec3f>& outVertices, std::vector<Maths::Vec3i>& outFaces);







