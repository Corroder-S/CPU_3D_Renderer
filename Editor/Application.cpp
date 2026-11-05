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


    m_mesh_1 = LoadMesh(ASSETS_PATH "Models/fantasy-diamond.obj");

    m_mesh_2 = LoadMesh(ASSETS_PATH "Models/ball-bearing.obj");




    MaterialComponent diamondMatComp;
    diamondMatComp.materials.reserve(m_mesh_1.submeshes.size());
    for (const auto& submesh : m_mesh_1.submeshes) {
        diamondMatComp.materials.push_back(submesh.material);
    }

    MaterialComponent ballMatComp;
    ballMatComp.materials.reserve(m_mesh_2.submeshes.size());
    for (const auto& submesh : m_mesh_2.submeshes) {
        ballMatComp.materials.push_back(submesh.material);
    }

    Transform transformOpaque;
    transformOpaque.position = { -1.0f, 0.0f, -20.0f };
	transformOpaque.scale = { 30.0f, 30.0f, 30.0f };
    auto entityOpaque = m_registry.createEntity();
    m_registry.addComponents(entityOpaque, transformOpaque, MeshComponent{ &m_mesh_2 }, ballMatComp);

    Transform transformTransp;
    transformTransp.position = { 0.0f, 0.0f, -3.0f };
    transformTransp.scale = { 30.0f, 30.0f, 30.0f };
    m_DiamondEntity = m_registry.createEntity();
    m_registry.addComponents(m_DiamondEntity, transformTransp, MeshComponent{ &m_mesh_1 }, diamondMatComp);

    TerrainParams terrainParams;
    terrainParams.width = 128;
    terrainParams.depth = 128;
    terrainParams.cellSize = 0.5f;
    terrainParams.heightScale = 12.0f;
    terrainParams.frequency = 0.02f;
    terrainParams.octaves = 10;
    terrainParams.persistence = 0.5f;
    terrainParams.lacunarity = 2.0f;
    terrainParams.warpStrength = 1.4f;
    terrainParams.seed = 2382425242;

    terrainParams.type = TerrainType::MountainRidged;

    m_terrainMesh = TerrainGenerator::Generate(terrainParams);

    Material terrainMaterial;
    terrainMaterial.ambient = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    terrainMaterial.diffuse = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    terrainMaterial.specular = Color{ 0.1f, 0.1f, 0.1f, 1.0f };
    terrainMaterial.shininess = 2.0f;

    MaterialComponent terrainMatComp;
    terrainMatComp.materials.push_back(terrainMaterial);


    auto terrainEntity = m_registry.createEntity();
    m_registry.addComponents(terrainEntity,
        Transform{
            .position = { 0.0f, -4.0f, 0.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 1.0f, 1.0f, 1.0f }
        },
        MeshComponent{ &m_terrainMesh },
        terrainMatComp
    );

    Light sun;
    sun.direction = Maths::Normalize(Maths::Vec3f{ 1.0f, -1.0f, -1.0f });
    sun.ambient = 0.4f;
    auto lightEntity = m_registry.createEntity();
    m_registry.addComponents(lightEntity, LightComponent{ sun });

    auto camEntity = m_registry.createEntity();
    auto& camera = m_registry.addComponent<CameraComponent>(camEntity, CameraComponent{ Camera{} });
    camera.camera.position = { 0.0f, 4.0f, 1.0f };
    camera.camera.target = { 0.0f, -1.0f, -5.0f };
    m_registry.addComponent<ActiveCameraTag>(camEntity, ActiveCameraTag{});

    m_particleManager.initialize(10000, 4);


    EmitterConfig config = loadFromJson(ASSETS_PATH "Particles/fire.json");

    auto* sys = m_particleManager.createSystem(config, { 0, 0, 0 });
    auto particleEntity = m_registry.createEntity();
    m_registry.addComponent<ParticleSystemComponent>(particleEntity, ParticleSystemComponent{ sys });

    m_renderer.setParticleManager(&m_particleManager);
    m_rendererSystem = std::make_unique<RendererSystem>(m_registry, m_renderer);
    m_uiSystem = std::make_unique<UISystem>(m_registry);

    m_FpsTextEntity = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(m_FpsTextEntity, Text{ 15, 15, &m_font, Color::Green, "FPS: 0", 100 });

    auto titleEntity = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(titleEntity, Text{ 15, 40, &m_font, Color::White, "Les tenebres", 100 });

    auto uiCanvaEntity = m_registry.createEntity();
    m_registry.addComponent<CanvaComponent>(uiCanvaEntity, Canva{ 20, 80, 220, 160, 0xAA111111, 0 });

    auto uiText1 = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(uiText1, Text{ 30, 90, &m_font, Color::White, "Scene Info", 1 });

    auto uiText2 = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(uiText2, Text{ 30, 115, &m_font, Color{0.7f, 0.7f, 0.7f, 1.0f}, "Terrain: 128x128", 1 });

    auto uiText3 = m_registry.createEntity();
    m_registry.addComponent<TextComponent>(uiText3, Text{ 30, 140, &m_font, Color{0.7f, 0.7f, 0.7f, 1.0f}, "Meshes: Diamond + Terrain", 1 });

    m_uiImage = std::make_shared<Texture>();
    if (m_uiImage->load(ASSETS_PATH "Image/EmperorOfManKind.jpg"))
    {
        auto imgEntity = m_registry.createEntity();
        m_registry.addComponent<ImageComponent>(imgEntity, Image{ 20, 260, 150, 150, m_uiImage.get(), 0 });
    }
}

void Application::update()
{
    float dt = Time::deltaTime();

    Camera* activeCamera = nullptr;
    m_registry.forEach<CameraComponent, ActiveCameraTag>([&](ECS::EntityID, CameraComponent& cam, ActiveCameraTag&)
        {
            activeCamera = &cam.camera;
        });
    if (!activeCamera) return;
    m_cameraController.update(*activeCamera, dt);

    auto& transform = m_registry.getComponent<Transform>(m_DiamondEntity);
    transform.rotation.y += dt;

    m_rendererSystem->update(dt);
    int fps = (dt > 0.0f) ? static_cast<int>(1.0f / dt) : 0;
    m_registry.getComponent<TextComponent>(m_FpsTextEntity).text.text = "FPS: " + std::to_string(fps);

    m_uiSystem->update(
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