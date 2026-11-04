#pragma once
#include "Camera.h"
#include "Asset/Light.h"
#include "Asset/Material.h"
#include "Asset/Mesh.h"
#include "Maths/Vec3.h"
#include "Renderer/Emitter/EmitterConfig.h"



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
    particleSystem* system = nullptr;
};

struct CameraComponent
{
    Camera camera;
};

struct ActiveCameraTag {};

