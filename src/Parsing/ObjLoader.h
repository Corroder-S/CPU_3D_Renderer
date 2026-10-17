#pragma once

#include <vector>

#include "Maths/Vec3.h"
#include "Maths/Vec2.h"
#include "Asset/Mesh.h"


bool LoadObj(const char* path, std::vector<Maths::Vec3f>& outVertices, std::vector<Maths::Vec2f>& outuvs, std::vector<face>& outFaces);







