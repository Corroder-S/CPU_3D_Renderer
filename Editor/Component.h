#pragma once
#include "Asset/Light.h"
#include "Asset/Material.h"
#include "Asset/Mesh.h"
#include "Maths/Vec3.h"
#include "Renderer/Emitter/EmitterConfig.h"
#include "Renderer/Emitter/ParticleSystem.h"
#include "Engine/Transform.h"
#include "UI/UICanvas.h"
#include "Engine/Camera.h"


struct MeshComponent
{
    Mesh* mesh = nullptr;
};

struct MaterialComponent
{
    std::vector<Material> materials;
};

struct LightComponent
{
    Light light;
};

struct ParticleSystemComponent
{
    ParticleSystem* system = nullptr;
};

struct CameraComponent
{
    Camera camera;
};

struct ActiveCameraTag {};

struct TransformComponent
{
    Transform transform;
};

struct CanvaComponent
{
    Canva canva;
};

struct TextComponent
{
    Text text;
};

struct ImageComponent
{
    Image image;
};