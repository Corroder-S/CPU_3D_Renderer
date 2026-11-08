#pragma once
#include "ParticleEmitter.h"

// ============================================================================
// SYSTÈME DE PARTICULES
// ============================================================================

/// @brief Regroupe plusieurs ParticleEmitter formant un effet visuel cohérent.
/// Un système est complet quand tous ses émetteurs non-looping ont terminé.
class ParticleSystem
{
    std::vector<ParticleEmitter*> m_emitters; ///< Émetteurs appartenant à ce système.
    std::uint32_t m_poolID;                   ///< Identifiant du pool mémoire parent.

public:
    ParticleSystem() = default;

    /// @brief Met à jour tous les émetteurs du système.
    /// @param deltaTime Temps écoulé depuis la dernière frame (secondes).
    void update(float deltaTime);

    /// @brief Alloue et ajoute un nouvel émetteur configuré par config.
    void addEmitter(EmitterConfig config);

    /// @brief Retire et libère l'émetteur donné du système.
    void removeEmitter(ParticleEmitter* emitter);

    void setPoolID(std::uint32_t poolID) { m_poolID = poolID; }

    /// @brief Réinitialise tous les émetteurs du système.
    void reset();

    /// @brief Retourne vrai si tous les émetteurs non-looping ont fini d'émettre.
    bool isComplete() const;

    /// @brief Retourne vrai si au moins un émetteur est en mode boucle.
    bool isLooping() const;

    const std::vector<ParticleEmitter*>& getEmitters() const { return m_emitters; }
};
