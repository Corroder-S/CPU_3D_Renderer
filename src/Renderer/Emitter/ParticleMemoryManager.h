#pragma once
#include <vector>

#include "ParticleMemoryPools.h"

class ParticleMemoryManager
{
    static ParticleMemoryManager* m_instance;
    std::vector<ParticleMemoryPool*> m_memoryPools;

public:
    void Initialize(uint32_t numParticles, uint32_t numEmitters, uint32_t numSystems, uint32_t numPools);
    static ParticleMemoryManager& GetInstance();
    ~ParticleMemoryManager();
    ParticleMemoryPool* getMemoryPool(uint32_t index);
    uint32_t getBestPoolIdForNewParticles() const;
    uint32_t getNumActiveParticles() const;
};