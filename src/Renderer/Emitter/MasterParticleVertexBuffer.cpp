//
// Created by leoam on 23/10/2026.
//

#include "MasterParticleVertexBuffer.h"

void MasterParticleVertexBuffer::initialize(size_t maxParticles, size_t numPools)
{
    uint32_t VertsPerPool = static_cast<uint32_t>(maxParticles / numPools);


    m_particleVerts.resize(numPools);
    for (auto& pool : m_particleVerts)
    {
        pool.resize(VertsPerPool);
        m_currentVerts.push_back(0);
    }
}

void MasterParticleVertexBuffer::clearParticleVertices()
{
    for (auto& vert : m_currentVerts)
    {
        vert = 0;
    }
}

void MasterParticleVertexBuffer::addParticleVertex(ParticleVertex *vertex, uint32_t poolIndex)
{
    uint32_t index = m_currentVerts[poolIndex];
    m_particleVerts[poolIndex][index] = *vertex;
    m_currentVerts[poolIndex]++;
}

uint32_t MasterParticleVertexBuffer::getTotalVertCount() const
{
    uint32_t total = 0;
    for (auto count : m_currentVerts)
        total += count;
    return total;
}

void MasterParticleVertexBuffer::mergeAllPools()
{
    m_vertexBuffer.clear();
    for (size_t i = 0; i < m_particleVerts.size(); i++)
    {
        auto& verts = m_particleVerts[i];
        uint32_t count = m_currentVerts[i];
        m_vertexBuffer.insert(m_vertexBuffer.end(), verts.begin(), verts.begin() + count);
    }
}
