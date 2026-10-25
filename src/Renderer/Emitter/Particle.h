#pragma once
#include "Maths/Vec3.h"


struct Particle
{
    Maths::Vec3f position;
    Maths::Vec3f velocity;
    float life = 0.0f;
    float startLife = 0.0f;
    float size = 1.0f;
    float startSize = 1.0f;
    float endSize = 0.0f;
    uint32_t color = 0xFFFFFFFF;
    uint32_t startColor = 0xFFFFFFFF;
    uint32_t endColor = 0xFFFFFFFF;

    void reset();
};
