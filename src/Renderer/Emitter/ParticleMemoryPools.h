#pragma once
#include <cstdint>
#include <vector>


#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"


class ParticleMemoryPool
{

    uint32_t m_maxParticles;
    uint32_t m_maxParticleEmitters;
    uint32_t m_maxParticleSystems;
    uint32_t m_currentParticleIndex;
    uint32_t m_currentEmitterIndex;
    uint32_t m_currentSystemIndex;

    ParticleSystem* m_particleSystems;
    ParticleEmitter* m_particleEmitters;
    Particle* m_particles;

    std::vector<Particle*> m_freeParticles;
    std::vector<ParticleEmitter*> m_freeEmitters;
    std::vector<ParticleSystem*> m_freeSystems;

public:
    ParticleMemoryPool() = default;
    ParticleMemoryPool(uint32_t numParticles, uint32_t numEmitters, uint32_t numSystem);
    ~ParticleMemoryPool();


    Particle* getNewParticle();
    ParticleEmitter* getNewEmitter();
    ParticleSystem* getNewSystem();


    void freeParticle(Particle *particle);
    void freeEmitter(ParticleEmitter* emitter);
    void freeSystem(ParticleSystem* system);

    uint32_t getNumActiveParticles() const;
};


