#pragma once
#include "Renderer/TerrainGenerator.h"
#include "Asset/Texture.h"
#include "Asset/Material.h"
#include "Scene/RenderObject.h"

#include <SDL3/SDL.h>
#include <memory>

class TerrainEditor {
public:
    TerrainParams params;
    Mesh terrainMesh;
    RenderObject terrainObject;
    std::shared_ptr<Texture> terrainTexture;

    void init(const std::string& texturePath) {
        terrainTexture = std::make_shared<Texture>();
        terrainTexture->load(texturePath);

        Material mat;
        mat.texture = terrainTexture;
        mat.ambient = Color{ 0.3f, 0.3f, 0.3f, 1.0f };
        mat.diffuse = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        mat.specular = Color{ 0.0f, 0.0f, 0.0f, 1.0f };

        terrainObject.material = mat;
        rebuild();
    }

    void rebuild() {
        terrainMesh = TerrainGenerator::Generate(params);
        terrainObject.mesh = &terrainMesh;
    }

    bool handleInput(const SDL_Event& event) {
        if (event.type == SDL_EVENT_KEY_DOWN) {
            switch (event.key.key) {
            case SDLK_KP_PLUS:
            case SDLK_EQUALS:
                params.heightScale += 1.0f;
                rebuild();
                return true;
            case SDLK_KP_MINUS:
            case SDLK_MINUS:
                params.heightScale = std::max(0.0f, params.heightScale - 1.0f);
                rebuild();
                return true;
            case SDLK_R:
                params.seed += 1;
                rebuild();
                return true;
            }
        }
        return false;
    }
};