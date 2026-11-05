#include "ParticleSystem.h"

#include "ParticleMemoryManager.h"

void ParticleSystem::update(float deltaTime)
{
    for (auto& emitter : m_emitters)
    {
        emitter->update(deltaTime);
        emitter->freeDeadParticles();
    }
}

void ParticleSystem::addEmitter(EmitterConfig config)
{
    m_emitters.emplace_back(ParticleMemoryManager::getInstance().getMemoryPool(m_poolID)->getNewEmitter());
    m_emitters.back()->setPoolId(m_poolID);
    m_emitters.back()->setConfig(config);
}

void ParticleSystem::removeEmitter(ParticleEmitter* emitter)
{
    std::erase_if(m_emitters,
        [emitter](const ParticleEmitter* e)
        {
            return e == emitter;
        });
}

bool ParticleSystem::isComplete() const
{
    for (auto& emitter : m_emitters)
    {
        if (!emitter->isDone())
            return false;
    }
    return true;
}

bool ParticleSystem::isLooping() const
{
    for (auto& emitter : m_emitters)
    {
        return emitter->isLoop();
    }
    return false;
}

void ParticleSystem::reset()
{
    auto pool = ParticleMemoryManager::getInstance().getMemoryPool(m_poolID);
    for (auto& emitter : m_emitters)
    {
        pool->freeEmitter(emitter);
    }
    m_emitters.clear();
    m_poolID = 0;
}


