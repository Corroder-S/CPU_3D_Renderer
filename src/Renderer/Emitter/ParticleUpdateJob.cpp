//
// Created by leoam on 23/10/2026.
//

#include "ParticleUpdateJob.h"
#include "ParticleMemoryManager.h"

void ParticleUpdateJob::execute()
{

    for (int i = 0; i < static_cast<int>(ParticleSystems->size()); i++)
    {
        particleSystem* system = (*ParticleSystems)[i];
        system->update(deltaTime);

        if (!system->isLooping() && system->isComplete())
        {
            auto* pool = ParticleMemoryManager::GetInstance().getMemoryPool(poolIndex);
            pool->FreeSystem(system);
            ParticleSystems->erase(ParticleSystems->begin() + i);
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
                masterBuffer->AddParticleVertex(&vert, poolIndex);
            }
        }
    }
}
