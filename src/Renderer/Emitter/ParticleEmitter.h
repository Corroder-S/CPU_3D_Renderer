#pragma once

#include <vector>

#include "Maths/Vec3.h"
#include "Particle.h"
#include "EmitterConfig.h"

// ============================================================================
// ÉMETTEUR DE PARTICULES
// ============================================================================

/// @brief Émetteur autonome qui gère un pool local de particules actives/libres.
/// Les pointeurs de particules sont alloués par le ParticleMemoryPool associé.
class ParticleEmitter
{
public:

    ParticleEmitter() = default;
    /// @brief Construit l'émetteur avec une configuration initiale.
    ParticleEmitter(const EmitterConfig& config);

    /// @brief Marque la particule la plus ancienne comme morte et la libère.
    void destroyParticle();
    /// @brief Émet une ou plusieurs particules selon le taux configuré.
    void emit();
    /// @brief Met à jour toutes les particules actives (physique, couleur, taille).
    /// @param deltaTime Temps écoulé depuis la dernière frame (secondes).
    void update(float deltaTime);
    /// @brief Déplace les particules mortes de la liste active vers la liste libre.
    void freeDeadParticles();

    void setPoolId(uint32_t poolId) { m_poolId = poolId; }
    void setConfig(const EmitterConfig& config) { m_config = config; }
    /// @brief Remet l'émetteur à zéro (libère toutes les particules actives).
    void reset();
    /// @brief Retourne vrai si aucune particule n'est actuellement active.
    bool isDone() const { return m_activeParticles.empty(); }
    bool isLoop() const { return m_config.looping; }
    const std::vector<Particle*>& getActiveParticles() const { return m_activeParticles; }

private:
    std::vector<Particle*> m_activeParticles; ///< Particules actuellement en vie.
    std::vector<Particle*> m_freeParticles;   ///< Particules disponibles pour réutilisation.
    uint32_t m_poolId = 0;                    ///< Identifiant du pool mémoire parent.
    Maths::Vec3f m_position;                  ///< Position de l'émetteur dans l'espace monde.
    EmitterConfig m_config;
    float m_emitAccumulator = 0.0f;           ///< Accumulation fractionnaire entre deux émissions.
};


