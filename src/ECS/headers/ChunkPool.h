#pragma once
#include <vector>

constexpr size_t CHUNK_SIZE = 16 * 1024;

struct ChunkPool
{
    std::vector<std::byte*> m_freeChunk;

    std::byte* allocate()
    {
        if (!m_freeChunk.empty())
        {
            std::byte* chunk = m_freeChunk.back();
            m_freeChunk.pop_back();
            return chunk;
        }
        return new std::byte[CHUNK_SIZE];
    }

    void freeChunk(std::byte* chunk)
    {
        m_freeChunk.push_back(chunk);
    }

    ~ChunkPool()
    {
        for (auto* chunk : m_freeChunk)
        {
            delete[] chunk;
        }
    }
};


struct Chunk
{
    std::byte* m_chunk = nullptr;
    uint32_t m_count = 0;
    uint32_t m_capacity = 0;
};