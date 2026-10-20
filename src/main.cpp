#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Engine/Engine.h"
#include "Engine/Time.h"
#include "Parsing/ObjLoader.h"
#include "Scene/Scene.h"

class MyApp : public Engine
{
    Scene   m_scene;
    Mesh    m_mesh;
    Texture m_texture;

    void start() override
    {
        m_mesh = LoadMesh(ASSETS_PATH "spot.obj");
        m_texture.load(ASSETS_PATH "spot_texture.png");

        RenderObject spot;
        spot.mesh    = &m_mesh;
        spot.texture = &m_texture;
        spot.transform.rotation.x = 3.14f;
        m_scene.objects.push_back(spot);

        m_scene.lights.push_back(Light{});

        m_scene.camera.position = { 0.0f, 0.0f, 5.0f };
        m_scene.camera.target   = { 0.0f, 0.0f, 0.0f };
    }

    void update() override
    {
        float t = SDL_GetTicks() / 1000.0f;
        m_scene.objects[0].transform.rotation.y = t;

        m_renderer.renderScene(m_scene);
    }
};

int main(int argc, char* argv[])
{
    MyApp app;
    return app.run();
}
