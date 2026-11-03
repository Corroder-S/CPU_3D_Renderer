#pragma once
#include "Asset/Material.h"
#include "Maths/Vec3.h"

struct TransformComponent
{
    Maths::Vec3f position = { 0.0f, 0.0f, 0.0f };
    Maths::Vec3f rotation = { 0.0f, 0.0f, 0.0f }; // euler angles (radians)
    Maths::Vec3f scale    = { 1.0f, 1.0f, 1.0f };
};


struct MeshComponent
{
    std::string fileName;
    std::string materialName;
};


struct MaterialComponent
{
    Material material;
};

struct EmitterComponent
{
    std::string emitterConfigFile;
};

