#pragma once

// ============================================================================
// INTERFACE SYSTÈME ECS
// ============================================================================

/// @brief Interface de base pour tous les systèmes ECS.
///
/// Chaque système concret implémente update() pour traiter
/// les entités correspondant à sa requête de composants.
struct ISystem
{
    /// @brief Exécute la logique du système pour la frame courante.
    /// @param deltaTime Temps écoulé depuis la frame précédente, en secondes.
    virtual void update(float deltaTime) = 0;
    virtual ~ISystem() = default;
};
