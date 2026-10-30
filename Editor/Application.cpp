#include "Application.h"
#include "Renderer/TerrainGenerator.h"

#include <execution>

#include "Engine/Time.h"
#include "Renderer/Emitter/ParticleMemoryManager.h"

void Application::start()
{
    RenderObject spot;

    if (!m_font.load(ASSETS_PATH "fonts/Paladins.ttf", 20.0f)) {
        SDL_Log("Erreur lors du chargement de la police !");
    }

    Material loadedMaterial;
    m_mesh = LoadMesh(ASSETS_PATH "car1.obj", loadedMaterial);


    RenderObject carOpaque;
    carOpaque.mesh = &m_mesh;
    carOpaque.material = loadedMaterial;
    carOpaque.material.dissolve = 1.0f;
    carOpaque.transform.position = { -1.0f, 0.0f, -10.0f };
    m_scene.objects.push_back(carOpaque);

    RenderObject carTransp;
    carTransp.mesh = &m_mesh;
    carTransp.material = loadedMaterial;
    carTransp.material.dissolve = 0.6f;
    carTransp.transform.position = { 0.0f, 0.0f, -5.0f };
    m_scene.objects.push_back(carTransp);

    TerrainParams terrainParams;
    terrainParams.width = 64;
    terrainParams.depth = 64;
    terrainParams.cellSize = 0.5f;
    terrainParams.heightScale = 4.0f;
    terrainParams.frequency = 0.04f;
    terrainParams.octaves = 4;

    m_terrainMesh = TerrainGenerator::Generate(terrainParams);

    RenderObject terrain;
    terrain.mesh = &m_terrainMesh;
    terrain.material.diffuse = Color{ 0.2f, 0.6f, 0.25f, 1.0f };
    terrain.material.ambient = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    terrain.material.specular = Color{ 0.2f, 0.2f, 0.2f, 1.0f };
    terrain.material.shininess = 4.0f;
    terrain.material.dissolve = 1.0f;
    terrain.transform.position = { 0.0f, -2.0f, 0.0f };
    m_scene.objects.push_back(terrain);

    Light sun;
    sun.direction = Maths::Normalize(Maths::Vec3f{ 1.0f, -1.0f, -1.0f });
    sun.ambient = 0.4f;
    m_scene.lights.push_back(sun);

    m_scene.camera.position = { 0.0f, 4.0f, 1.0f };
    m_scene.camera.target = { 0.0f, -1.0f, -5.0f };

    m_particleManager.initialize(10000, 4);

    emitterConfig cfg = loadFromJson(ASSETS_PATH "fire.json");
    m_particleManager.createSystem(cfg, { 0, 0, 0 });
    m_renderer.setParticleManager(&m_particleManager);
}

void Application::update()
{
    float dt = Time::deltaTime();

    m_cameraController.update(m_scene.camera, dt);

    m_scene.objects[1].transform.rotation.y += dt;

    m_renderer.renderScene(m_scene);

    int fps = (dt > 0.0f) ? static_cast<int>(1.0f / dt) : 0;
    m_renderer.drawText(m_font, "FPS: " + std::to_string(fps), 15, 15, Color::Green);
    m_renderer.drawText(m_font, "Les tenebres", 15, 40, Color::White);
    m_particleManager.update(dt);

    auto viewProj = m_scene.camera.getProjMatrix((float)m_renderer.getWidth() / m_renderer.getHeight())
        * m_scene.camera.getViewMatrix();

    auto& verts = m_particleManager.getMasterBuffer().getVertexBuffer();
    uint32_t count = m_particleManager.getMasterBuffer().getTotalVertCount();
    std::sort(std::execution::par_unseq, verts.begin(), verts.end(),
        [&](ParticleVertex& a, ParticleVertex& b) {
            auto da = a.position - m_scene.camera.position;
            auto db = b.position - m_scene.camera.position;
            float distA = da.x * da.x + da.y * da.y + da.z * da.z;
            float distB = db.x * db.x + db.y * db.y + db.z * db.z;
            return distA > distB;
        });
    for (uint32_t i = 0; i < count; i++)
    {
        m_renderer.drawParticle(verts[i], viewProj);
    }
}