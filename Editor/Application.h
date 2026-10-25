#pragma once
#include "Asset/Light.h"
#include "Asset/Mesh.h"
#include "Asset/Texture.h"
#include "Asset/Font.h"
#include "Engine/Engine.h"
#include "Scene/RenderObject.h"
#include "Scene/Scene.h"
#include "SDL3/SDL_timer.h"

class Application : public Engine
{
    Scene   m_scene;
    Mesh    m_mesh;
	Font    m_font;
    std::shared_ptr<Texture> m_texture;
    ParticleManager m_particleManager;



public:
    void start()  override;
    void update() override;
};
