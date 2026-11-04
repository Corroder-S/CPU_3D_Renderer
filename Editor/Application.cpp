#include "Application.h"
#include "Renderer/TerrainGenerator.h"

#include <execution>

#include "Engine/Time.h"
#include "Renderer/Emitter/ParticleMemoryManager.h"
#include "Component.h"

void Application::start()
{

    if (!m_font.load(ASSETS_PATH "fonts/paladins.ttf", 20.0f)) {
        SDL_Log("Erreur lors du chargement de la police !");
    }

    Material loadedMaterial;
    m_mesh = LoadMesh(ASSETS_PATH "Models/Car1.obj", loadedMaterial);


    RenderObject carOpaque;
    carOpaque.mesh = &m_mesh;
    carOpaque.material = loadedMaterial;
    carOpaque.material.dissolve = 1.0f;
    carOpaque.transform.position = { -1.0f, 0.0f, -10.0f };
    auto entity = m_registry.createEntity();
    m_registry.addComponents(entity, carOpaque.transform, MeshComponent{ carOpaque.mesh }, MaterialComponent{ carOpaque.material });



    RenderObject carTransp;
    carTransp.mesh = &m_mesh;
    carTransp.material = loadedMaterial;
    carTransp.material.dissolve = 0.6f;
    carTransp.transform.position = { 0.0f, 0.0f, -5.0f };
    m_carTranspEntity = m_registry.createEntity();
    m_registry.addComponents(m_carTranspEntity, carTransp.transform, MeshComponent{ carTransp.mesh }, MaterialComponent{ carTransp.material });

    TerrainParams terrainParams;
    terrainParams.width = 64;
    terrainParams.depth = 64;
    terrainParams.cellSize = 0.5f;
    terrainParams.heightScale = 8.0f;
    terrainParams.frequency = 0.1f;
    terrainParams.octaves = 8;
    terrainParams.seed = 65565;

    m_terrainMesh = TerrainGenerator::Generate(terrainParams);

    RenderObject terrain;
    terrain.mesh = &m_terrainMesh;
    terrain.material.diffuse = Color{ 0.2f, 0.6f, 0.25f, 1.0f };
    terrain.material.ambient = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    terrain.material.specular = Color{ 0.2f, 0.2f, 0.2f, 1.0f };
    terrain.material.shininess = 4.0f;
    terrain.material.dissolve = 1.0f;
    terrain.transform.position = { 0.0f, -2.0f, 0.0f };
    auto terrainEntity = m_registry.createEntity();
    m_registry.addComponents(terrainEntity,
        Transform{ .position = { 0.0f, -2.0f, 0.0f } },
        MeshComponent{ &m_terrainMesh },
        MaterialComponent{ terrain.material });

    Light sun;
    sun.direction = Maths::Normalize(Maths::Vec3f{ 1.0f, -1.0f, -1.0f });
    sun.ambient = 0.4f;
    entity = m_registry.createEntity();
    m_registry.addComponents(entity, LightComponent{ sun });


    entity = m_registry.createEntity();
    auto& camera = m_registry.addComponent<CameraComponent>(entity, CameraComponent{ Camera{} });
    camera.camera.position = { 0.0f, 4.0f, 1.0f };
    camera.camera.target = { 0.0f, -1.0f, -5.0f };
    m_registry.addComponent<ActiveCameraTag>(entity, ActiveCameraTag{});

    m_particleManager.initialize(10000, 4);


    emitterConfig config = loadFromJson(ASSETS_PATH "Particles/fire.json");

    auto* sys = m_particleManager.createSystem(config, { 0, 0, 0 });
    entity = m_registry.createEntity();
    m_registry.addComponent<ParticleSystemComponent>(entity, ParticleSystemComponent{ sys });

    m_renderer.setParticleManager(&m_particleManager);
    m_rendererSystem = std::make_unique<RendererSystem>(m_registry, m_renderer);
    m_uiSystem = std::make_unique<UISystem>(m_registry);

    m_FpsTextEntity = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(m_FpsTextEntity, Text{15, 15, &m_font, Color::Green, "FPS: 0", 100});
    entity = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(entity, Text{15, 40, &m_font, Color::White, "Les tenebres", 100});

    // Canvas de debug UI
    entity = m_registry.createEntity();
    m_registry.addComponent<CanvaComponent>(entity, Canva{20, 80, 220, 160, 0xAA111111, 0});

    entity = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(entity, Text{30, 90, &m_font, Color::White, "Scene Info", 1});

    entity = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(entity, Text{30, 115, &m_font, Color{0.7f, 0.7f, 0.7f, 1.0f}, "Terrain: 64x64", 1});

    entity = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(entity, Text{30, 140, &m_font, Color{0.7f, 0.7f, 0.7f, 1.0f}, "Meshes: Car + Terrain", 1});

    // Image
    m_uiImage = std::make_shared<Texture>();
    if (m_uiImage->load(ASSETS_PATH "Image/EmperorOfManKind.jpg"))
    {
        entity = m_registry.createEntity();
        m_registry.addComponent<ImageComponent>(entity, Image{20, 260, 150, 150, m_uiImage.get(), 0});
    }

}

void Application::update()
{
    float dt = Time::deltaTime();

    Camera *activeCamera = nullptr;
    m_registry.forEach<CameraComponent, ActiveCameraTag>([&](ECS::EntityID, CameraComponent& cam, ActiveCameraTag&)
    {
        activeCamera = &cam.camera;
    });
    if (!activeCamera) return;
    m_cameraController.update(*activeCamera, dt);


    auto& transform = m_registry.getComponent<Transform>(m_carTranspEntity);
    transform.rotation.y += dt;


    m_rendererSystem->update(dt);
    int fps = (dt > 0.0f) ? static_cast<int>(1.0f / dt) : 0;
    m_registry.getComponent<TextComponent>(m_FpsTextEntity).text.text = "FPS: " + std::to_string(fps);
    m_uiSystem->update(                  // UI par-dessus
        m_renderer.getFrameBuffer(),
        m_renderer.getWidth(),
        m_renderer.getHeight()
    );


    m_particleManager.update(dt);

    auto viewProj = activeCamera->getProjMatrix((float)m_renderer.getWidth() / m_renderer.getHeight())
    * activeCamera->getViewMatrix();

    auto& verts = m_particleManager.getMasterBuffer().getVertexBuffer();
    uint32_t count = m_particleManager.getMasterBuffer().getTotalVertCount();
    std::sort(std::execution::par_unseq, verts.begin(), verts.end(),
        [&](ParticleVertex& a, ParticleVertex& b) {
            auto da = a.position - activeCamera->position;
            auto db = b.position - activeCamera->position;
            float distA = da.x * da.x + da.y * da.y + da.z * da.z;
            float distB = db.x * db.x + db.y * db.y + db.z * db.z;
            return distA > distB;
        });
    for (uint32_t i = 0; i < count; i++)
    {
        m_renderer.drawParticle(verts[i], viewProj);
    }
}