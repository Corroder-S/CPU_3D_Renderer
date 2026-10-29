#pragma once
#include "ParticleSystem.h"
#include "MasterParticleVertexBuffer.h"
#include "WorkerPool.h"

class ParticleManager
{
    std::vector<std::vector<particleSystem*>> m_systemsPerPool;
    MasterParticleVertexBuffer m_masterBuffer;
    uint32_t m_numPools = 0;
    WorkerPool m_workers;
public:
    void initialize(uint32_t maxParticles, uint32_t numPools);
    void createSystem(const emitterConfig& config, const Maths::Vec3f& position);
    void destroySystem(particleSystem* system);
    void update(float deltaTime);
     MasterParticleVertexBuffer& getMasterBuffer()  { return m_masterBuffer; }
    const std::vector<std::vector<particleSystem*>>& getSystemsPerPool() const { return m_systemsPerPool; }
};
