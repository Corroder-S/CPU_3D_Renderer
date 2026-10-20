#pragma once

#include "Maths/Vec3.h"
#include "Maths/Matrix4x4.h"

struct Camera
{
    Maths::Vec3f position = { 0.0f, 0.0f, 5.0f };
    Maths::Vec3f target   = { 0.0f, 0.0f, 0.0f };
    Maths::Vec3f up       = { 0.0f, 1.0f, 0.0f };

    float fov   = 1.047f;
    float nearZ = 0.1f;
    float farZ  = 100.0f;

    Maths::Matrix4x4<> getViewMatrix() const
    {
        return Maths::LookAt(position, target, up);
    }

    Maths::Matrix4x4<> getProjMatrix(float aspectRatio) const
    {
        return Maths::Perspective(fov, aspectRatio, nearZ, farZ);
    }
};
