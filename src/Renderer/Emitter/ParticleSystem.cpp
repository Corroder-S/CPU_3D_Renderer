#include "ParticleSystem.h"

#include "ParticleMemoryManager.h"

void particleSystem::update(float deltaTime)
{
    for (auto& emitter : particleEmitters)
    {
        emitter->update(deltaTime);
        emitter->freeDeadParticles();
    }
}

void particleSystem::addEmitter(emitterConfig config)
{
    particleEmitters.emplace_back(ParticleMemoryManager::GetInstance().getMemoryPool(m_poolID)->GetNewEmitter());
    particleEmitters.back()->setPoolId(m_poolID);
    particleEmitters.back()->setConfig(config);
}

void particleSystem::removeEmitter(particleEmitter* emitter)
{
    std::erase_if(particleEmitters,
        [emitter](const particleEmitter* e)
        {
            return e == emitter;
        });
}

bool particleSystem::isComplete() const
{
    for (auto& emitter : particleEmitters)
    {
        if (!emitter->isDone())
            return false;
    }
    return true;
}

bool particleSystem::isLooping() const
{
    for (auto& emitter : particleEmitters)
    {
        return emitter->isLoop();
    }
    return false;
}

void particleSystem::reset()
{
    auto pool = ParticleMemoryManager::GetInstance().getMemoryPool(m_poolID);
    for (auto& emitter : particleEmitters)
    {
        pool->FreeEmitter(emitter);
    }
    particleEmitters.clear();
    m_poolID = 0;
}


