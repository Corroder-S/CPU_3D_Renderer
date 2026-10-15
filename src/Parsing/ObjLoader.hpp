#pragma once

#include <vector>
#include <array>
#include "Maths/Vec3.h"
#include "Maths/Vec2.h"


bool LoadObj(const char* filename, std::vector<Maths::Vec3<>>& out_vertices, std::vector<Maths::Vec3<>>>& out_faces);
