#pragma once
#include "Asset/Light.h"
#include "Asset/Mesh.h"
#include "Asset/Texture.h"
#include "Engine/Engine.h"
#include "Scene/RenderObject.h"
#include "Scene/Scene.h"
#include "SDL3/SDL_timer.h"

class Application : public Engine
{
    Scene   m_scene;
    Mesh    m_mesh;
    Texture m_texture;

public:
    void start() override;


    void update() override;

};
