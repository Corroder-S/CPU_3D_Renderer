#pragma once

#include <vector>

#include "Maths/Vec3.h"
#include "Particle.h"
#include "EmitterConfig.h"


class particleEmitter
{
public:

    particleEmitter() = default;
    particleEmitter(const emitterConfig& config);

    void destroyParticle();
    void emit();
    void update(float deltaTime);
    void freeDeadParticles();
    void setPoolId(uint32_t poolId) { m_poolId = poolId; }
    void setConfig(const emitterConfig& config) { m_config = config; }
    void reset();
    bool isDone() const { return m_activeParticles.empty(); }
    bool isLoop() const { return m_config.looping; }
    const std::vector<Particle*>& getActiveParticles() const { return m_activeParticles; }

private:
    std::vector<Particle*> m_activeParticles;
    std::vector<Particle*> m_freeParticles;
    uint32_t m_poolId = 0;
    Maths::Vec3f Position;
    emitterConfig m_config;
    float m_emitAccumulator = 0.0f;


};


