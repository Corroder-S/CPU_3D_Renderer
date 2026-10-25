//
// Created by leoam on 23/10/2026.
//

#include "ParticleMemoryManager.h"

ParticleMemoryManager* ParticleMemoryManager::m_instance = nullptr;

void ParticleMemoryManager::Initialize(uint32_t numParticles, uint32_t numEmitters, uint32_t numSystems,
    uint32_t numPools)
{
    for (uint32_t i = 0; i < numPools; i++)
    {
        m_memoryPools.push_back(new ParticleMemoryPool(numParticles/numPools, numEmitters/numPools, numSystems/numPools));
    }
}

ParticleMemoryManager* ParticleMemoryManager::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new ParticleMemoryManager();
    }
    return m_instance;
}

ParticleMemoryPool* ParticleMemoryManager::getMemoryPool(uint32_t index)
{
    if (index < m_memoryPools.size())
    {
        return m_memoryPools[index];
    }
    return nullptr;
}


uint32_t ParticleMemoryManager::getBestPoolIdForNewParticles() const
{
    uint32_t bestId = 0;
    uint32_t lowestParticles = m_memoryPools[0]->GetNumActiveParticles();
    for (uint32_t i = 0; i < m_memoryPools.size(); i++)
    {
        uint32_t numParticles = m_memoryPools[i]->GetNumActiveParticles();
        if (numParticles < lowestParticles)
        {
            bestId = i;
            lowestParticles = numParticles;
        }
    }
    return bestId;
}

uint32_t ParticleMemoryManager::getNumActiveParticles() const
{
    uint32_t totalActiveParticles = 0;
    for (auto& pool : m_memoryPools)
    {
        totalActiveParticles += pool->GetNumActiveParticles();
    }
    return totalActiveParticles;
}
