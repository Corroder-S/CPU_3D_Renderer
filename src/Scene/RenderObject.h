#pragma once

#include "Transform.h"
#include "../Asset/Mesh.h"
#include "../Asset/Texture.h"

struct RenderObject
{
    Mesh*     mesh    = nullptr;
    Texture*  texture = nullptr;
    Transform transform;
};
