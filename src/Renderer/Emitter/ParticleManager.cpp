#include "ParticleManager.h"
#include "ParticleMemoryManager.h"
#include "ParticleUpdateJob.h"
#include <algorithm>
#include <thread>

void ParticleManager::initialize(uint32_t maxParticles, uint32_t numPools)
{
    m_numPools = numPools;
    ParticleMemoryManager::getInstance().initialize(
        maxParticles,
        maxParticles / 10,
        maxParticles / 100,
        numPools
    );
    m_masterBuffer.initialize(maxParticles, numPools);
    m_systemsPerPool.resize(numPools);
}

ParticleSystem* ParticleManager::createSystem(const EmitterConfig& config, const Maths::Vec3f& position)
{
    auto& memoryManager = ParticleMemoryManager::getInstance();
    const uint32_t poolID = memoryManager.getBestPoolIdForNewParticles();
    auto pool = memoryManager.getMemoryPool(poolID);
    ParticleSystem* system = pool->getNewSystem();

    if (!system)
    {
        assert(false && "Failed to create particle system: pool is full");
        return nullptr;
    }

    system->setPoolID(poolID);
    system->addEmitter(config);
    m_systemsPerPool[poolID].push_back(system);
    return system;
}

void ParticleManager::destroySystem(ParticleSystem* system)
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
            job.particleSystems = &m_systemsPerPool[i];
            job.masterBuffer = &m_masterBuffer;
            job.poolIndex = i;
            job.deltaTime = deltaTime;
            job.execute();
        });

    m_masterBuffer.mergeAllPools();
}