#pragma once
#include <vector>

#include "Maths/Vec3.h"

// ============================================================================
// VERTEX DE PARTICULE
// ============================================================================

/// @brief Données GPU d'une particule billboardée (position + couleur + taille).
struct ParticleVertex
{
    Maths::Vec3f position; ///< Centre du billboard dans l'espace monde.
    uint32_t color;        ///< Couleur ARGB 32 bits.
    float size;            ///< Demi-taille du billboard.
};

// ============================================================================
// BUFFER MAÎTRE DE VERTICES DE PARTICULES
// ============================================================================

/// @brief Agrège les vertices de particules produits par tous les pools en un
/// buffer de rendu contigu, en conservant un sous-buffer par pool pour
/// permettre les écritures parallèles sans contention.
class MasterParticleVertexBuffer
{
    std::vector<std::vector<ParticleVertex>> m_particleVerts; ///< Sous-buffer par pool.
    std::vector<uint32_t> m_currentVerts;                     ///< Nombre de vertices actifs par pool.
    std::vector<uint32_t> m_vertOffsetsForPools;              ///< Offset de chaque pool dans le buffer final.

    std::vector<ParticleVertex> m_vertexBuffer; ///< Buffer contigu fusionné, envoyé au renderer.
    std::vector<uint32_t> m_indexBuffer;

public:
    /// @brief Alloue les sous-buffers pour le nombre maximal de particules et de pools.
    /// @param maxParticles Capacité totale en vertices.
    /// @param numPools Nombre de pools (et de sous-buffers).
    void initialize(size_t maxParticles, size_t numPools);

    /// @brief Remet à zéro les compteurs de vertices de chaque pool (sans désalloquer).
    void clearParticleVertices();

    /// @brief Ajoute un vertex dans le sous-buffer du pool indiqué (thread-safe par pool).
    /// @param vertex Vertex à copier.
    /// @param poolIndex Index du pool émetteur.
    void addParticleVertex(ParticleVertex* vertex, uint32_t poolIndex);

    /// @brief Fusionne tous les sous-buffers dans m_vertexBuffer de façon contiguë.
    void mergeAllPools();

    const std::vector<ParticleVertex>& getVertexBuffer() const { return m_vertexBuffer; }
    std::vector<ParticleVertex>& getVertexBuffer() { return m_vertexBuffer; }

    /// @brief Retourne le nombre total de vertices actifs après fusion.
    uint32_t getTotalVertCount() const;
};


