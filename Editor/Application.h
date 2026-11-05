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


class Application : public Engine
{
    Mesh             m_mesh_1;
    Mesh             m_mesh_2;
    Mesh             m_terrainMesh;
    Font             m_font;
    std::shared_ptr<Texture> m_texture;
    std::shared_ptr<Texture> m_uiImage;
    ParticleManager  m_particleManager;
    CameraController m_cameraController{ 10.0f };

    std::unique_ptr<RendererSystem> m_rendererSystem;
    std::unique_ptr<UISystem> m_uiSystem;
    ECS::Registry m_registry;

    ECS::EntityID m_DiamondEntity;
    ECS::EntityID m_FpsTextEntity;

public:
    void start()  override;
    void update() override;
};
