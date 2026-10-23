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

    Material loadedMaterial;
    m_mesh = LoadMesh(ASSETS_PATH "Car1.obj", loadedMaterial);

    RenderObject carOpaque;
    carOpaque.mesh = &m_mesh;
    carOpaque.material = loadedMaterial;
    carOpaque.material.dissolve = 1.0f;
    carOpaque.transform.position = { -1.0f, 0.0f, -10.0f };
    m_scene.objects.push_back(carOpaque);

    RenderObject carTransp;
    carTransp.mesh = &m_mesh;
    carTransp.material = loadedMaterial;
    carTransp.material.dissolve = 0.5f;
    carTransp.transform.position = { 0.0f, 0.0f, -5.0f };
    m_scene.objects.push_back(carTransp);

    m_scene.lights.push_back(Light{});
    m_scene.camera.position = { 0.0f, 0.0f, 0.0f };

    m_scene.camera.position = { 0.0f, 0.0f, 3.0f };
    m_scene.camera.target = { 0.0f, 0.0f, 0.0f };
}

void Application::update()
{
    float dt = Time::deltaTime();
    m_scene.objects[1].transform.rotation.y += dt;

    m_renderer.renderScene(m_scene);


    int fps = (dt > 0.0f) ? static_cast<int>(1.0f / dt) : 0;
    m_renderer.drawText(m_font, "FPS: " + std::to_string(fps), 15, 15, Color::Green);
    m_renderer.drawText(m_font, "Les tenebres", 15, 40, Color::White);
}