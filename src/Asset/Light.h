#pragma once

#include "Maths/Vec3.h"

// ============================================================================
// LIGHTING STRUCTURES
// ============================================================================

/// @brief Représente une source de lumière directionnelle globale (ex: le Soleil) avec composante ambiante.
struct Light
{
    Maths::Vec3f direction = Maths::Normalize(Maths::Vec3f(1.0f, 1.0f, -1.0f)); /// Vectoriel d'orientation de la lumière (normalisé).
    float ambient = 0.18f;                                                       /// Intensité de la lumière ambiante minimale.
};