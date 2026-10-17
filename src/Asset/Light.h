#pragma once

#include "Maths/Vec3.h"
struct Light
{
    Maths::Vec3f direction = Maths::Normalize(Maths::Vec3f(1.0f, 1.0f, -1.0f));
    float ambient = 0.18f;
};