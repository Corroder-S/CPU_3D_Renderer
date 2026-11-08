#pragma once
#include <vector>

#include "ChunkPool.h"
#include "ComponentMetaRegistry.h"
#include "entity_component.h"

// ============================================================================
// ARCHÉTYPE ECS — STOCKAGE PAR COLONNES DE CHUNKS
// ============================================================================

/// @brief Colonne de données pour un seul type de composant dans un archétype.
///
/// Les données sont stockées dans une liste de Chunks contigus alloués
/// depuis un ChunkPool partagé.
struct Column
{
    std::vector<Chunk*> m_Data;
    size_t stride = 0;

    /// @brief Retourne le chunk courant, ou en alloue un nouveau s'il est plein.
    /// @param pool          Pool de chunks à utiliser pour l'allocation.
    /// @param chunkCapacity Nombre maximum d'éléments par chunk.
    /// @return Pointeur vers le chunk disponible en écriture.
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

/// @brief Groupe d'entités partageant exactement le même ensemble de composants.
///
/// Chaque archétype possède une colonne d'entités et une colonne par type
/// de composant. La capacité d'un chunk est calculée à la création.
struct Archetypes
{
    Column m_entityColumn;
    std::vector<Column> m_componentData;
    /// @brief Liste ordonnée des IDs de types de composants qui définissent cet archétype.
    std::vector<uint32_t> m_componentTypeList;
    /// @brief Table de correspondance typeID → index de colonne dans m_componentData.
    std::vector<uint32_t> componentToColumn;
    uint32_t chunkCapacity;
};

/// @brief Localisation d'une entité dans son archétype.
struct EntityRecord
{
    Archetypes* archetype;
    uint32_t chunkIndex;
    uint32_t slotIndex;
};
