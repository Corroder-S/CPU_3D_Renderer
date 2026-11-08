#pragma once

#include "Maths/Vec3.h"
#include "Maths/Matrix4x4.h"

// ============================================================================
// CAMERA
// ============================================================================

/// @brief Caméra en perspective avec paramètres de vue et de projection.
struct Camera
{
    Maths::Vec3f position = { 0.0f, 0.0f, 5.0f };
    Maths::Vec3f target   = { 0.0f, 0.0f, 0.0f };
    Maths::Vec3f up       = { 0.0f, 1.0f, 0.0f };

    float fov   = 1.5708f; // Champ de vision vertical (radians) — ~90°
    float nearZ = 0.1f;    // Plan de coupe proche
    float farZ  = 100.0f;  // Plan de coupe lointain

    /// @brief Calcule la matrice de vue (LookAt) à partir de la position et de la cible.
    /// @return Matrice 4x4 de vue.
    Maths::Matrix4x4<> getViewMatrix() const
    {
        return Maths::LookAt(position, target, up);
    }

    /// @brief Calcule la matrice de projection perspective.
    /// @param aspectRatio Rapport largeur/hauteur de la fenêtre.
    /// @return Matrice 4x4 de projection.
    Maths::Matrix4x4<> getProjMatrix(float aspectRatio) const
    {
        return Maths::Perspective(fov, aspectRatio, nearZ, farZ);
    }
};
