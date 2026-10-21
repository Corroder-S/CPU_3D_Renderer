#pragma once

#include "Transform.h"
#include "Asset/Mesh.h"
#include "Asset/Material.h"

struct RenderObject
{
    Mesh*     mesh    = nullptr;
    Material material;
    Transform transform;
};
