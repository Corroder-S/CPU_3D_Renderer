#include "ParticleMemoryPools.h"

ParticleMemoryPool::ParticleMemoryPool(uint32_t numParticles, uint32_t numEmitters, uint32_t numSystem)
{
    m_maxParticles = numParticles;
    m_maxParticleEmitters = numEmitters;
    m_maxParticleSystems = numSystem;

    m_currentParticleIndex = 0;
    m_currentEmitterIndex = 0;
    m_currentSystemIndex = 0;

    m_particles = new Particle[m_maxParticles];
    m_particleEmitters = new particleEmitter[m_maxParticleEmitters];
    m_particleSystems = new particleSystem[m_maxParticleSystems];

    m_freeParticles.reserve(m_maxParticles);
    m_freeEmitters.reserve(m_maxParticleEmitters);
    m_freeSystems.reserve(m_maxParticleSystems);
}

ParticleMemoryPool::~ParticleMemoryPool()
{
    delete[] m_particles;
    delete[] m_particleEmitters;
    delete[] m_particleSystems;
}

Particle *ParticleMemoryPool::GetNewParticle()
{
    Particle *newParticle = nullptr;

    if (m_freeParticles.empty())
    {

    }
    if (m_currentParticleIndex < m_maxParticles)
    {
        newParticle = &m_particles[m_currentParticleIndex];
        m_currentParticleIndex++;
    }
    else if (!m_freeParticles.empty())
    {
        newParticle = m_freeParticles.back();
        m_freeParticles.pop_back();
    }
    return newParticle;
}

particleEmitter * ParticleMemoryPool::GetNewEmitter()
{
    particleEmitter *newEmitter = nullptr;

    if (m_currentEmitterIndex < m_maxParticleEmitters)
    {
        newEmitter = &m_particleEmitters[m_currentEmitterIndex];
        m_currentEmitterIndex++;
    }
    else if (!m_freeEmitters.empty())
    {
        newEmitter = m_freeEmitters.back();
        m_freeEmitters.pop_back();
    }
    return newEmitter;
}

particleSystem * ParticleMemoryPool::GetNewSystem()
{
    particleSystem *newSystem = nullptr;

    if (m_currentSystemIndex < m_maxParticleSystems)
    {
        newSystem = &m_particleSystems[m_currentSystemIndex];
        m_currentSystemIndex++;
    }
    else if (!m_freeSystems.empty())
    {
        newSystem = m_freeSystems.back();
        m_freeSystems.pop_back();
    }
    return newSystem;
}




void ParticleMemoryPool::FreeParticle(size_t *particle, bool loop)
{
    assert(particle != nullptr);
    if (!loop)
    {
        m_freeParticles.push_back(reinterpret_cast<Particle*>(particle));
    }
}

void ParticleMemoryPool::FreeEmitter(particleEmitter *emitter)
{
    assert(emitter != nullptr);
    emitter->reset();
    m_freeEmitters.push_back(emitter);
}


void ParticleMemoryPool::FreeSystem(particleSystem *system)
{
    assert(system != nullptr);
    system->reset();
    m_freeSystems.push_back(system);
}

uint32_t ParticleMemoryPool::GetNumActiveParticles() const
{
    return m_currentParticleIndex - m_freeParticles.size();
}
