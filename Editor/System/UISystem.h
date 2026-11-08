#pragma once
#include "ECS/headers/registry.h"
#include "UI/UICanvas.h"

// ============================================================================
// SYSTÈME D'INTERFACE UTILISATEUR ECS
// ============================================================================

/// @brief Système ECS qui dessine les éléments UI (textes, images) sur le framebuffer.
///
/// Itère sur les entités possédant CanvaComponent, TextComponent ou ImageComponent
/// et délègue le rendu à l'UICanvas interne.
class UISystem
{
    ECS::Registry&  m_registry;
    UICanvas m_ui;
public:
    /// @brief Construit le système avec une référence au registry.
    UISystem(ECS::Registry& registry) : m_registry(registry) {}

    /// @brief Dessine tous les éléments UI sur le framebuffer fourni.
    /// @param framebuffer Pointeur vers le tampon de pixels destination.
    /// @param fbWidth     Largeur du framebuffer en pixels.
    /// @param fbHeight    Hauteur du framebuffer en pixels.
    void update(uint32_t* framebuffer, uint32_t fbWidth, uint32_t fbHeight);
};
