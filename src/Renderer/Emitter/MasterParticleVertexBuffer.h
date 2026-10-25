#pragma once
#include <vector>

#include "Maths/Vec3.h"

struct ParticleVertex
{
    Maths::Vec3f position;
    uint32_t color;
    float size;
};

class MasterParticleVertexBuffer
{
    std::vector<std::vector<ParticleVertex>> m_ParticleVerts;
    std::vector<uint32_t> m_CurrentVerts;
    std::vector<uint32_t> m_vertOffsetsForPools;

    std::vector<ParticleVertex> m_vertexBuffer;
    std::vector<uint32_t> m_indexBuffer;

public:
    void Initialize(size_t maxParticles, size_t numPools);
    void clearParticleVertices();
    void AddParticleVertex(ParticleVertex* vertex, uint32_t poolIndex);
    void mergeAllPools();

    const std::vector<ParticleVertex>& getVertexBuffer() const { return m_vertexBuffer; }
    std::vector<ParticleVertex>& getVertexBuffer() { return m_vertexBuffer; }
    uint32_t getTotalVertCount() const;
};


