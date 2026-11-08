#pragma once

#include "Maths/Vec3.h"
#include "Maths/Matrix4x4.h"

// ============================================================================
// TRANSFORM
// ============================================================================

/// @brief Représente la transformation locale d'un objet dans le monde (TRS).
struct Transform
{
    Maths::Vec3f position = { 0.0f, 0.0f, 0.0f };
    Maths::Vec3f rotation = { 0.0f, 0.0f, 0.0f }; // angles d'Euler (radians)
    Maths::Vec3f scale    = { 1.0f, 1.0f, 1.0f };

    /// @brief Calcule la matrice modèle (Translation × Rotation × Scale).
    /// @return Matrice 4x4 du modèle dans l'espace monde.
    Maths::Matrix4x4<> getModelMatrix() const
    {
        return Maths::Model(position, rotation, scale);
    }
};
