#pragma once
#include "Scene/Camera.h"
#include "Asset/Light.h"
#include "Asset/Material.h"
#include "Asset/Mesh.h"
#include "Maths/Vec3.h"
#include "Renderer/Emitter/EmitterConfig.h"
#include "Renderer/Emitter/ParticleSystem.h"
#include "Scene/Transform.h"
#include "UI/UI_Canva.h"


struct MeshComponent
{
    Mesh* mesh = nullptr;
};

struct MaterialComponent
{
    Material material;
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