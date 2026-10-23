//
// Created by leoam on 20/10/2026.
//

#include "Application.h"
#include "Engine/Time.h"


void Application::start()
{
    RenderObject spot;

    if (!m_font.load(ASSETS_PATH "fonts/Paladins.ttf", 20.0f)) {
        SDL_Log("Erreur lors du chargement de la police !");
    }

    m_mesh = LoadMesh(ASSETS_PATH "Car1.obj", spot.material);

    Maths::Vec3f centroid = { 0, 0, 0 };
    for (auto& v : m_mesh.vertices) centroid = centroid + v;
    centroid = centroid * (1.0f / (float)m_mesh.vertices.size());
    for (auto& v : m_mesh.vertices) v = v - centroid;

    spot.mesh = &m_mesh;

    if (!spot.material.texture) {
        spot.material.texture = m_texture;
    }

    spot.transform.position = { 0.0f, 0.0f, -5.0f };
    m_scene.objects.push_back(spot);

    m_scene.lights.push_back(Light{});

    m_scene.camera.position = { 0.0f, 0.0f, 3.0f };
    m_scene.camera.target = { 0.0f, 0.0f, 0.0f };
}

void Application::update()
{
    float dt = Time::deltaTime();
    m_scene.objects[0].transform.rotation.y += dt;

    m_renderer.renderScene(m_scene);


    int fps = (dt > 0.0f) ? static_cast<int>(1.0f / dt) : 0;
    m_renderer.drawText(m_font, "FPS: " + std::to_string(fps), 15, 15, Color::Green);
    m_renderer.drawText(m_font, "Les tenebres", 15, 40, Color::White);
}