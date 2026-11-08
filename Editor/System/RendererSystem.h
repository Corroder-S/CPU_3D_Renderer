#pragma once
#include "ECS/headers/ISystem.h"
#include "ECS/headers/registry.h"
#include "Renderer/Renderer.h"

// ============================================================================
// SYSTÈME DE RENDU ECS
// ============================================================================

/// @brief Système ECS qui soumet les meshes et les lumières au Renderer chaque frame.
///
/// Itère sur les entités possédant MeshComponent + TransformComponent
/// (et optionnellement MaterialComponent) et délègue le rendu au Renderer.
class RendererSystem : public ISystem
{
    ECS::Registry& m_registry;
    Renderer&      m_renderer;

public:
    /// @brief Construit le système avec les références partagées au registry et au renderer.
    RendererSystem(ECS::Registry& registry, Renderer& renderer)
        : m_registry(registry), m_renderer(renderer) {}

    /// @brief Soumet tous les meshes visibles au renderer pour la frame courante.
    /// @param deltaTime Temps écoulé depuis la frame précédente, en secondes.
    void update(float deltaTime) override;

    Renderer &getRenderer() { return m_renderer; }
};
