#pragma once
#include "Maths/Vec3.h"

// ============================================================================
// PARTICULE
// ============================================================================

/// @brief Représente l'état d'une particule individuelle.
/// Allouée et gérée par un ParticleMemoryPool.
struct Particle
{
    Maths::Vec3f position;           ///< Position dans l'espace monde.
    Maths::Vec3f velocity;           ///< Vitesse courante.
    float life      = 0.0f;          ///< Temps de vie restant (secondes).
    float startLife = 0.0f;          ///< Durée de vie totale à la naissance.
    float size      = 1.0f;          ///< Taille courante.
    float startSize = 1.0f;          ///< Taille initiale.
    float endSize   = 0.0f;          ///< Taille en fin de vie.
    uint32_t color      = 0xFFFFFFFF; ///< Couleur courante (format 0xAARRGGBB).
    uint32_t startColor = 0xFFFFFFFF; ///< Couleur initiale.
    uint32_t endColor   = 0xFFFFFFFF; ///< Couleur en fin de vie.

    /// @brief Remet la particule dans son état initial (prête à être réutilisée).
    void reset();
};
