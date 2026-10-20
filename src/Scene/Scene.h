#pragma once

#include <vector>
#include "Camera.h"
#include "RenderObject.h"
#include "../Asset/Light.h"

struct Scene
{
    std::vector<RenderObject> objects;
    std::vector<Light>        lights;
    Camera                    camera;
};
