#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <iostream>

#include "Maths/Vec3.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Draw.h"
#include "Renderer/Pipeline.h"
#include "Parsing/ObjLoader.h"


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

    const Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 last = SDL_GetPerformanceCounter();
    double fps_accum = 0.0;
    int    fps_count = 0;
    char   title[160];

    std::vector<uint32_t> framebuffer(RENDER_WIDTH * RENDER_HEIGHT);
    std::vector<float>    zbuffer(RENDER_WIDTH * RENDER_HEIGHT);

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    bool isRunning = true;
    SDL_Event event;

    std::vector<Maths::Vec3f> vertices;
    std::vector<face> faces;
    std::vector<Maths::Vec2f> uvs;
    LoadObj(ASSETS_PATH "spot.obj", vertices, uvs, faces);

    Texture AssetTexture;
    AssetTexture.load(ASSETS_PATH "spot_texture.png");



    std::cout << "vertices: " << vertices.size() << std::endl;
    std::cout << "uvs: "    << uvs.size()    << std::endl;
    std::cout << "faces: "    << faces.size()    << std::endl;




    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                isRunning = false;
        }

        clear(rgb(16, 16, 24), framebuffer);
        std::fill(zbuffer.begin(), zbuffer.end(), 1e30f);

        float t = SDL_GetTicks() / 1000.0f;


        Maths::Matrix4x4<> model = Maths::Model(
        Maths::Vec3<>(0.0f, 1.0f, 0.0f),  // descend le modèle
    Maths::Vec3<>(3.14f, t, 0.0f),
    Maths::Vec3<>(1.0f, 1.0f, 1.0f)
);
        Maths::Matrix4x4<> view = Maths::LookAt(
            Maths::Vec3<>(0.0f, 0.0f, -5.0f),  // caméra plus loin
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

        std::vector<Maths::Vec2i> projected(vertices.size());
        std::vector<float>        depths(vertices.size());
        std::vector<float>        invWs(vertices.size());
        projectMesh(MVP, vertices.data(), vertices.size(), projected.data(), depths.data(), invWs.data());

        for (auto& f : faces)
        {
            Maths::Vec3f a = { (float)projected[f.v.x].x, (float)projected[f.v.x].y, depths[f.v.x] };
            Maths::Vec3f b = { (float)projected[f.v.y].x, (float)projected[f.v.y].y, depths[f.v.y] };
            Maths::Vec3f c = { (float)projected[f.v.z].x, (float)projected[f.v.z].y, depths[f.v.z] };

            int area = edge(a, b, c);
            if (area >= 0) continue;

            Maths::Vec3f invW = { invWs[f.v.x], invWs[f.v.y], invWs[f.v.z] };


            drawTriangle(a, b, c, uvs[f.uv.x], uvs[f.uv.y], uvs[f.uv.z], invW, framebuffer, zbuffer, AssetTexture);
        }


        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), RENDER_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);



        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - last) / (double)freq;
        last = now;
        fps_accum += dt;
        fps_count += 1;
        if (fps_accum >= 1.0) {
            double fps = (double)fps_count / fps_accum;
            double ms = 1000.0 * fps_accum / (double)fps_count;
            snprintf(title, sizeof title, "CPU 3D RENDERER  |  %.1f FPS  (%.2f ms/frame)", fps, ms);
            SDL_SetWindowTitle(window, title);
            fps_accum = 0.0;
            fps_count = 0;
        }
    }


    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
