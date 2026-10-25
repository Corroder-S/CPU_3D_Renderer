#pragma once
#include <vector>

#include "MasterParticleVertexBuffer.h"
#include "ParticleSystem.h"

class ParticleUpdateJob
{
public:

    std::vector<particleSystem*>* ParticleSystems;
    MasterParticleVertexBuffer* masterBuffer;
    uint32_t poolIndex;

    float deltaTime;
    Maths::Vec3<float> cameraDirection;
    Maths::Vec3<float> cameraPosition;

    virtual void execute();
};
