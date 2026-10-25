#pragma once
#include "particleEmitter.h"

class particleSystem
{
    std::vector<particleEmitter*> particleEmitters;
    std::uint32_t m_poolID;
public:
    particleSystem() = default;
    void update(float deltaTime);
    void addEmitter(emitterConfig config);
    void removeEmitter(particleEmitter* emitter);
    void setPoolID(std::uint32_t poolID) { m_poolID = poolID; }
    void reset();
    bool isComplete() const;
    bool isLooping() const;
    const std::vector<particleEmitter*>& getEmitters() const { return particleEmitters; }

};
