#include "particleEmitter.h"

#include <random>

#include "ParticleMemoryManager.h"
#include "Maths/Utils.h"

static std::mt19937 rng(std::random_device{}());

particleEmitter::particleEmitter(const emitterConfig &config)
{
    m_config = config;
}

void particleEmitter::destroyParticle()
{
    m_activeParticles.clear();
}

void particleEmitter::emit()
{
    Particle* p = ParticleMemoryManager::GetInstance()->getMemoryPool(m_poolId)->GetNewParticle();
    if (p == nullptr) return;
    p->position  = Position;

    constexpr float DEG2RAD = 3.14159265f / 180.0f;

    std::uniform_real_distribution<float> distXY(m_config.angleMin, m_config.angleMax);
    float angleXY = distXY(rng) * DEG2RAD;
    std::uniform_real_distribution<float> distZ(-m_config.spreadAngle, m_config.spreadAngle);
    float angleZ = distZ(rng) * DEG2RAD;


    p->velocity.x = cos(angleXY) * m_config.startSpeed;
    p->velocity.y = sin(angleXY) * m_config.startSpeed;
    p->velocity.z = sin(angleZ) * m_config.startSpeed;
    p->startLife  = m_config.maxlife;
    p->life       = m_config.maxlife;
    p->startSize  = m_config.startSize;
    p->endSize    = m_config.endSize;
    p->startColor = m_config.startColor;
    p->endColor   = m_config.endColor;
    m_activeParticles.push_back(p);
}

void particleEmitter::update(float deltaTime)
{
    m_emitAccumulator += deltaTime;
    float interval = 1.0f / m_config.emitRate;
    while (m_emitAccumulator >= interval)
    {
        emit();
        m_emitAccumulator -= interval;
    }


    for (auto& particle : m_activeParticles)
    {
        particle->velocity.y += -9.81f * m_config.gravityScale * deltaTime;
        particle->position += particle->velocity * deltaTime;
        particle->life     -= deltaTime;
        float timeStep = std::clamp(1.0f - (particle->life / particle->startLife), 0.0f, 1.0f);
        particle->size  = Maths::lerp(particle->startSize, particle->endSize, timeStep);
        particle->color = Maths::lerpColor(particle->startColor, particle->endColor, timeStep);
    }
}

void particleEmitter::freeDeadParticles()
{
    for (auto it = m_activeParticles.begin(); it != m_activeParticles.end();)
    {
        Particle* particle = *it;
        if (particle->life <= 0.0f)
        {
            ParticleMemoryManager::GetInstance()->getMemoryPool(m_poolId)->FreeParticle(particle);
            it = m_activeParticles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void particleEmitter::reset()
{
    m_activeParticles.clear();
    m_freeParticles.clear();
    m_poolId = 0;
    Position = Maths::Vec3f(0.0f, 0.0f, 0.0f);
    m_config = emitterConfig();
}
