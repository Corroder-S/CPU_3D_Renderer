//
// Created by leoam on 23/10/2026.
//

#include "ParticleUpdateJob.h"
#include "ParticleMemoryManager.h"

void ParticleUpdateJob::execute()
{

    for (int i = 0; i < static_cast<int>(particleSystems->size()); i++)
    {
        ParticleSystem* system = (*particleSystems)[i];
        system->update(deltaTime);

        if (!system->isLooping() && system->isComplete())
        {
            auto* pool = ParticleMemoryManager::getInstance().getMemoryPool(poolIndex);
            pool->freeSystem(system);
            particleSystems->erase(particleSystems->begin() + i);
            i--;
            continue;
        }

        if (system->isComplete())
        {
            continue;
        }

        for (auto* emitter : system->getEmitters())
        {
            for (auto* particle : emitter->getActiveParticles())
            {
                ParticleVertex vert;
                vert.position = particle->position;
                vert.color    = particle->color;
                vert.size     = particle->size;
                masterBuffer->addParticleVertex(&vert, poolIndex);
            }
        }
    }
}
