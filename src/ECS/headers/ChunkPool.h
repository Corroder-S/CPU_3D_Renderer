#pragma once
#include <vector>

// ============================================================================
// POOL DE CHUNKS MÉMOIRE
// ============================================================================

/// @brief Taille fixe d'un chunk en octets (16 Ko).
constexpr size_t CHUNK_SIZE = 16 * 1024;

/// @brief Allocateur pool pour blocs mémoire de taille fixe (CHUNK_SIZE).
///
/// Les chunks libérés sont conservés en liste pour être réutilisés,
/// évitant les allocations répétées sur le tas.
struct ChunkPool
{
    std::vector<std::byte*> m_freeChunk;

    /// @brief Retourne un chunk disponible ou en alloue un nouveau.
    /// @return Pointeur vers un bloc de CHUNK_SIZE octets.
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

    /// @brief Rend un chunk au pool pour réutilisation ultérieure.
    /// @param chunk Bloc à libérer (ne doit pas être nullptr).
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

/// @brief Bloc de données brutes avec métadonnées de remplissage.
struct Chunk
{
    std::byte* m_chunk = nullptr;
    uint32_t m_count = 0;
    uint32_t m_capacity = 0;
};
