#pragma once
#include <vector>

#include "MasterParticleVertexBuffer.h"
#include "ParticleSystem.h"

class ParticleUpdateJob
{
public:

    std::vector<ParticleSystem*>* particleSystems;
    MasterParticleVertexBuffer* masterBuffer;
    uint32_t poolIndex;

    float deltaTime;
    Maths::Vec3<float> cameraDirection;
    Maths::Vec3<float> cameraPosition;

    virtual void execute();
};
