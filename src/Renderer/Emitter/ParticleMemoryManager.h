#pragma once
#include <vector>

#include "ParticleMemoryPools.h"

// ============================================================================
// GESTIONNAIRE MÉMOIRE DE PARTICULES (SINGLETON)
// ============================================================================

/// @brief Singleton qui possède et distribue les ParticleMemoryPool.
/// Chaque pool correspond à un thread de mise à jour pour éviter
/// les contentions mémoire lors des allocations parallèles.
class ParticleMemoryManager
{
    static ParticleMemoryManager* m_instance;
    std::vector<ParticleMemoryPool*> m_memoryPools;

public:
    /// @brief Crée et initialise tous les pools.
    /// @param numParticles Nombre maximal de particules par pool.
    /// @param numEmitters Nombre maximal d'émetteurs par pool.
    /// @param numSystems Nombre maximal de systèmes par pool.
    /// @param numPools Nombre total de pools.
    void initialize(uint32_t numParticles, uint32_t numEmitters, uint32_t numSystems, uint32_t numPools);

    /// @brief Retourne l'instance unique du gestionnaire.
    static ParticleMemoryManager& getInstance();

    ~ParticleMemoryManager();

    /// @brief Retourne le pool à l'index donné.
    ParticleMemoryPool* getMemoryPool(uint32_t index);

    /// @brief Retourne l'index du pool ayant le moins de particules actives.
    uint32_t getBestPoolIdForNewParticles() const;

    /// @brief Retourne le nombre total de particules actives sur tous les pools.
    uint32_t getNumActiveParticles() const;
};
