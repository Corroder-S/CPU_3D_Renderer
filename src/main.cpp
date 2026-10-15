#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <iostream>

#include "Renderer/Framebuffer.h"
#include "Renderer/Draw.h"
#include "Renderer/Pipeline.h"


Maths::Vec3<> cubeVertices[8] = {
    { -0.5f, -0.5f, -0.5f },  // 0 back  bottom left
    {  0.5f, -0.5f, -0.5f },  // 1 back  bottom right
    {  0.5f,  0.5f, -0.5f },  // 2 back  top    right
    { -0.5f,  0.5f, -0.5f },  // 3 back  top    left
    { -0.5f, -0.5f,  0.5f },  // 4 front bottom left
    {  0.5f, -0.5f,  0.5f },  // 5 front bottom right
    {  0.5f,  0.5f,  0.5f },  // 6 front top    right
    { -0.5f,  0.5f,  0.5f },  // 7 front top    left
};

int cubeEdges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},  // back face
    {4,5}, {5,6}, {6,7}, {7,4},  // front face
    {0,4}, {1,5}, {2,6}, {3,7},  // connections
};


int main(int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("3D CPU Engine", 1280, 800, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        std::cerr << "Erreur création fenêtre : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH, RENDER_HEIGHT
    );

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    std::vector<uint32_t> framebuffer(RENDER_WIDTH * RENDER_HEIGHT);

    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                isRunning = false;
        }

        clear(rgb(16, 16, 24), framebuffer);

        float t = SDL_GetTicks() / 1000.0f;

        Maths::Matrix4x4<> model = Maths::Model(
            Maths::Vec3<>(0.0f, 0.0f, 0.0f),
            Maths::Vec3<>(0.0f, t,    0.0f),
            Maths::Vec3<>(1.0f, 1.0f, 1.0f)
        );
        Maths::Matrix4x4<> view = Maths::LookAt(
            Maths::Vec3<>(0.0f, 0.0f, -3.0f),
            Maths::Vec3<>(0.0f, 0.0f,  0.0f),
            Maths::Vec3<>(0.0f, 1.0f,  0.0f)
        );
        Maths::Matrix4x4<> proj = Maths::Perspective(
            1.047f,
            (float)RENDER_WIDTH / RENDER_HEIGHT,
            0.1f,
            100.0f
        );

        Maths::Matrix4x4<> MVP = proj * view * model;

        Maths::Vec2i projected[8];
        projectMesh(MVP, cubeVertices, 8, projected);

        for (auto& e : cubeEdges)
            drawline(projected[e[0]].x, projected[e[0]].y,
                     projected[e[1]].x, projected[e[1]].y,
                     rgb(255, 255, 255), framebuffer);

        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), RENDER_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
