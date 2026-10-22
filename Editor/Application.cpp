//
// Created by leoam on 20/10/2026.
//

#include "Application.h"

#include "Application.h"

void Application::start()
{
    m_mesh = LoadMesh(ASSETS_PATH "spot.obj");

    m_texture = std::make_shared<Texture>();
    m_texture->load(ASSETS_PATH "spot_texture.png");

    Maths::Vec3f centroid = { 0, 0, 0 };
    for (auto& v : m_mesh.vertices) centroid = centroid + v;
    centroid = centroid * (1.0f / (float)m_mesh.vertices.size());
    for (auto& v : m_mesh.vertices) v = v - centroid;

    RenderObject spot;
    spot.mesh = &m_mesh;
    spot.material.texture = m_texture;
    spot.material.diffuse = Color::White;
    spot.transform.position = { 0.0f, 0.0f, 1.0f };

    m_scene.objects.push_back(spot);


    m_scene.lights.push_back(Light{});

    m_scene.camera.position = { 0.0f, 0.0f, 3.0f };
    m_scene.camera.target = { 0.0f, 0.0f, 0.0f };
}

void Application::update()
{
    float t = SDL_GetTicks() / 1000.0f;
    m_scene.objects[0].transform.rotation.y = t;

    m_renderer.renderScene(m_scene);
}