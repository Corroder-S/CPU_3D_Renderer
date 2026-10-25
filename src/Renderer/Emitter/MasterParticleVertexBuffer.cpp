//
// Created by leoam on 23/10/2026.
//

#include "MasterParticleVertexBuffer.h"

void MasterParticleVertexBuffer::Initialize(size_t maxParticles, size_t numPools)
{
    uint32_t VertsPerPool = static_cast<uint32_t>(maxParticles / numPools);


    m_ParticleVerts.resize(numPools);
    for (auto& pool : m_ParticleVerts)
    {
        pool.resize(VertsPerPool);
        m_CurrentVerts.push_back(0);
    }
}

void MasterParticleVertexBuffer::clearParticleVertices()
{
    for (auto& vert : m_CurrentVerts)
    {
        vert = 0;
    }
}

void MasterParticleVertexBuffer::AddParticleVertex(ParticleVertex *vertex, uint32_t poolIndex)
{
    uint32_t index = m_CurrentVerts[poolIndex];
    m_ParticleVerts[poolIndex][index] = *vertex;
    m_CurrentVerts[poolIndex]++;
}

uint32_t MasterParticleVertexBuffer::getTotalVertCount() const
{
    uint32_t total = 0;
    for (auto count : m_CurrentVerts)
        total += count;
    return total;
}

void MasterParticleVertexBuffer::mergeAllPools()
{
    m_vertexBuffer.clear();
    for (size_t i = 0; i < m_ParticleVerts.size(); i++)
    {
        auto& verts = m_ParticleVerts[i];
        uint32_t count = m_CurrentVerts[i];
        m_vertexBuffer.insert(m_vertexBuffer.end(), verts.begin(), verts.begin() + count);
    }
}
