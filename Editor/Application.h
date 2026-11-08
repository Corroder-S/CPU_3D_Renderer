#pragma once
#include "Asset/Light.h"
#include "Asset/Mesh.h"
#include "Asset/Texture.h"
#include "Asset/Font.h"
#include "ECS/headers/registry.h"
#include "Engine/Engine.h"
#include "Engine/CameraController.h"
#include "SDL3/SDL_timer.h"
#include "System/RendererSystem.h"
#include "System/UISystem.h"

// ============================================================================
// APPLICATION PRINCIPALE DE L'ÉDITEUR
// ============================================================================

/// @brief Point d'entrée de la démo : initialise la scène et orchestre les systèmes.
///
/// Hérite de Engine pour accéder à la boucle principale, au renderer
/// et aux hooks start()/update().
class Application : public Engine
{
    Mesh             m_mesh;
    Mesh             m_terrainMesh;
    Font             m_font;
    std::shared_ptr<Texture> m_texture;
    std::shared_ptr<Texture> m_uiImage;
    ParticleManager  m_particleManager;
    CameraController m_cameraController{ 10.0f };

    std::unique_ptr<RendererSystem> m_rendererSystem;
    std::unique_ptr<UISystem> m_uiSystem;
    ECS::Registry m_registry;

    ECS::EntityID m_carTranspEntity;
    ECS::EntityID m_FpsTextEntity;

public:
    /// @brief Charge les assets, crée les entités et configure les systèmes.
    void start()  override;

    /// @brief Met à jour les systèmes et gère les entrées caméra.
    void update() override;
};
