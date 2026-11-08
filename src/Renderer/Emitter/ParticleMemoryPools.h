#pragma once
#include <cstdint>
#include <vector>


#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"

// ============================================================================
// POOL MÉMOIRE DE PARTICULES
// ============================================================================

/// @brief Pool d'allocation linéaire pour Particle, ParticleEmitter et ParticleSystem.
/// Les objets sont alloués dans des tableaux contigus pré-alloués ; les libérations
/// s'effectuent via des listes de pointeurs libres pour éviter la fragmentation.
class ParticleMemoryPool
{

    uint32_t m_maxParticles;         ///< Capacité maximale de particules.
    uint32_t m_maxParticleEmitters;  ///< Capacité maximale d'émetteurs.
    uint32_t m_maxParticleSystems;   ///< Capacité maximale de systèmes.
    uint32_t m_currentParticleIndex; ///< Prochain index libre dans m_particles.
    uint32_t m_currentEmitterIndex;  ///< Prochain index libre dans m_particleEmitters.
    uint32_t m_currentSystemIndex;   ///< Prochain index libre dans m_particleSystems.

    ParticleSystem*  m_particleSystems;  ///< Tableau contigu de systèmes pré-alloués.
    ParticleEmitter* m_particleEmitters; ///< Tableau contigu d'émetteurs pré-alloués.
    Particle*        m_particles;        ///< Tableau contigu de particules pré-allouées.

    std::vector<Particle*>       m_freeParticles; ///< Particules libérées, prêtes à réutiliser.
    std::vector<ParticleEmitter*> m_freeEmitters;  ///< Émetteurs libérés, prêts à réutiliser.
    std::vector<ParticleSystem*>  m_freeSystems;   ///< Systèmes libérés, prêts à réutiliser.

public:
    ParticleMemoryPool() = default;
    /// @brief Pré-alloue les tableaux pour les capacités données.
    ParticleMemoryPool(uint32_t numParticles, uint32_t numEmitters, uint32_t numSystem);
    ~ParticleMemoryPool();

    /// @brief Retourne une particule libre (depuis la liste libre ou par allocation linéaire).
    Particle* getNewParticle();
    /// @brief Retourne un émetteur libre.
    ParticleEmitter* getNewEmitter();
    /// @brief Retourne un système libre.
    ParticleSystem* getNewSystem();

    /// @brief Restitue une particule à la liste libre.
    void freeParticle(Particle *particle);
    /// @brief Restitue un émetteur à la liste libre.
    void freeEmitter(ParticleEmitter* emitter);
    /// @brief Restitue un système à la liste libre.
    void freeSystem(ParticleSystem* system);

    /// @brief Retourne le nombre de particules actuellement actives dans ce pool.
    uint32_t getNumActiveParticles() const;
};


