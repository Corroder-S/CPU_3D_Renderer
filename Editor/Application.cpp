//
// Created by leoam on 20/10/2026.
//

#include "Application.h"

void Application::start()
{
    m_mesh = LoadMesh(ASSETS_PATH "spot.obj");
    m_texture.load(ASSETS_PATH "spot_texture.png");

    RenderObject spot;
    spot.mesh    = &m_mesh;
    spot.texture = &m_texture;
    spot.transform.rotation.x = 3.14f;
    m_scene.objects.push_back(spot);

    m_scene.lights.push_back(Light{});

    m_scene.camera.position = { 0.0f, 0.0f, 5.0f };
    m_scene.camera.target   = { 0.0f, 0.0f, 0.0f };
}

void Application::update()
{
    float t = SDL_GetTicks() / 1000.0f;
    m_scene.objects[0].transform.rotation.y = t;

    m_renderer.renderScene(m_scene);
}