#pragma once
#include <vector>

#include "ChunkPool.h"
#include "ComponentMetaRegistry.h"
#include "entity_component.h"

struct Column
{
    std::vector<Chunk*> m_Data;
    size_t stride = 0;

    Chunk* getOrAddChunk(ChunkPool& pool, size_t chunkCapacity)
    {
        if (m_Data.empty() || m_Data.back()->m_count >= m_Data.back()->m_capacity)
        {
            Chunk* newChunk = new Chunk();
            newChunk->m_chunk = pool.allocate();
            newChunk->m_count = 0;
            newChunk->m_capacity = chunkCapacity;
            m_Data.push_back(newChunk);
        }
        return m_Data.back();
    }
};

struct Archetypes
{
    Column m_entityColumn;
    std::vector<Column> m_componentData;
    std::vector<uint32_t> m_componentTypeList;
    std::vector<uint32_t> componentToColumn;
    uint32_t chunkCapacity;
};


struct EntityRecord
{
    Archetypes* archetype;
    uint32_t chunkIndex;
    uint32_t slotIndex;
};


