#pragma once
#include <cstdint>
#include <vector>


#include "Particle.h"
#include "particleEmitter.h"
#include "ParticleSystem.h"


class ParticleMemoryPool
{

    uint32_t m_maxParticles;
    uint32_t m_maxParticleEmitters;
    uint32_t m_maxParticleSystems;
    uint32_t m_currentParticleIndex;
    uint32_t m_currentEmitterIndex;
    uint32_t m_currentSystemIndex;

    particleSystem* m_particleSystems;
    particleEmitter* m_particleEmitters;
    Particle* m_particles;

    std::vector<Particle*> m_freeParticles;
    std::vector<particleEmitter*> m_freeEmitters;
    std::vector<particleSystem*> m_freeSystems;

public:
    ParticleMemoryPool() = default;
    ParticleMemoryPool(uint32_t numParticles, uint32_t numEmitters, uint32_t numSystem);
    ~ParticleMemoryPool();


    Particle* GetNewParticle();
    particleEmitter* GetNewEmitter();
    particleSystem* GetNewSystem();


    void FreeParticle(Particle *particle);
    void FreeEmitter(particleEmitter* emitter);
    void FreeSystem(particleSystem* system);

    uint32_t GetNumActiveParticles() const;
};


