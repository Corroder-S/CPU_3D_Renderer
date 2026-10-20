#pragma once

#include "Maths/Vec3.h"
#include "Maths/Matrix4x4.h"

struct Transform
{
    Maths::Vec3f position = { 0.0f, 0.0f, 5.0f };
    Maths::Vec3f rotation = { 0.0f, 0.0f, 0.0f }; // euler angles (radians)
    Maths::Vec3f scale    = { 1.0f, 1.0f, 1.0f };

    Maths::Matrix4x4<> getModelMatrix() const
    {
        return Maths::Model(position, rotation, scale);
    }
};
