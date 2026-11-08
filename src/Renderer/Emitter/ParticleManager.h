#pragma once
#include "ParticleSystem.h"
#include "MasterParticleVertexBuffer.h"
#include "WorkerPool.h"

// ============================================================================
// GESTIONNAIRE DE PARTICULES
// ============================================================================

/// @brief Point d'entrée principal du système de particules.
/// Gère la création/destruction des ParticleSystem, distribue les mises à jour
/// sur plusieurs threads via WorkerPool et agrège les vertices dans le MasterBuffer.
class ParticleManager
{
    std::vector<std::vector<ParticleSystem*>> m_systemsPerPool; ///< Systèmes groupés par pool mémoire.
    MasterParticleVertexBuffer m_masterBuffer;                  ///< Buffer de rendu agrégé.
    uint32_t m_numPools = 0;
    WorkerPool m_workers;

public:
    /// @brief Initialise les pools mémoire et le buffer de rendu.
    /// @param maxParticles Nombre maximal de particules au total.
    /// @param numPools Nombre de pools (et de threads de mise à jour).
    void initialize(uint32_t maxParticles, uint32_t numPools);

    /// @brief Alloue un nouveau système de particules depuis le pool le moins chargé.
    /// @param config Configuration de l'émetteur initial du système.
    /// @param position Position monde du système.
    /// @return Pointeur vers le système créé.
    ParticleSystem* createSystem(const EmitterConfig& config, const Maths::Vec3f& position);

    /// @brief Libère le système et restitue sa mémoire au pool.
    void destroySystem(ParticleSystem* system);

    /// @brief Met à jour tous les systèmes en parallèle et reconstruit le MasterBuffer.
    /// @param deltaTime Temps écoulé depuis la dernière frame (secondes).
    void update(float deltaTime);

    MasterParticleVertexBuffer& getMasterBuffer()  { return m_masterBuffer; }
    const std::vector<std::vector<ParticleSystem*>>& getSystemsPerPool() const { return m_systemsPerPool; }
};
