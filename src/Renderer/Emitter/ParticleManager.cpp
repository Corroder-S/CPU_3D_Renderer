#include "ParticleManager.h"
#include "ParticleMemoryManager.h"
#include "ParticleUpdateJob.h"
#include <algorithm>
#include <thread>

void ParticleManager::initialize(uint32_t maxParticles, uint32_t numPools)
{
    m_numPools = numPools;
    ParticleMemoryManager::GetInstance().Initialize(
        maxParticles,
        maxParticles / 10,
        maxParticles / 100,
        numPools
    );
    m_masterBuffer.Initialize(maxParticles, numPools);
    m_systemsPerPool.resize(numPools);
}

void ParticleManager::createSystem(const emitterConfig& config, const Maths::Vec3f& position)
{
    auto& memoryManager = ParticleMemoryManager::GetInstance();
    const uint32_t poolID = memoryManager.getBestPoolIdForNewParticles();
    auto pool = memoryManager.getMemoryPool(poolID);
    particleSystem* system = pool->GetNewSystem();

    if (!system)
    {
        assert(false && "Failed to create particle system: pool is full");
        return;
    }

    system->setPoolID(poolID);
    system->addEmitter(config);
    m_systemsPerPool[poolID].push_back(system);
}

void ParticleManager::destroySystem(particleSystem* system)
{
    for (auto& list : m_systemsPerPool)
        list.erase(std::remove(list.begin(), list.end(), system), list.end());
}

void ParticleManager::update(float deltaTime)
{
    m_masterBuffer.clearParticleVertices();

    m_workers.parallelFor(m_numPools, [&](uint32_t i)
        {
            ParticleUpdateJob job;
            job.ParticleSystems = &m_systemsPerPool[i];
            job.masterBuffer = &m_masterBuffer;
            job.poolIndex = i;
            job.deltaTime = deltaTime;
            job.execute();
        });

    m_masterBuffer.mergeAllPools();
}