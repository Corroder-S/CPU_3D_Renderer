#pragma once
#include "ParticleEmitter.h"

class ParticleSystem
{
    std::vector<ParticleEmitter*> m_emitters;
    std::uint32_t m_poolID;
public:
    ParticleSystem() = default;
    void update(float deltaTime);
    void addEmitter(EmitterConfig config);
    void removeEmitter(ParticleEmitter* emitter);
    void setPoolID(std::uint32_t poolID) { m_poolID = poolID; }
    void reset();
    bool isComplete() const;
    bool isLooping() const;
    const std::vector<ParticleEmitter*>& getEmitters() const { return m_emitters; }

};
