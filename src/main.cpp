#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <iostream>

#include "Maths/Vec3.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Draw.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Clip.h"
#include "Parsing/ObjLoader.h"
#include "Asset/Light.h"




int main(int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("3D CPU Engine", RENDER_WIDTH, RENDER_HEIGHT, SDL_WINDOW_RESIZABLE);
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

    Mesh spot = LoadMesh(ASSETS_PATH "spot.obj");

    Light light;

    Texture AssetTexture;
    AssetTexture.load(ASSETS_PATH "spot_texture.png");

    std::cout << "vertices: " << spot.vertices.size() << std::endl;
    std::cout << "uvs: "    << spot.uvs.size()    << std::endl;
    std::cout << "faces: "    << spot.faces.size()    << std::endl;
    std::cout << "faceNormals: " << spot.faceNormals.size() << std::endl;
    std::cout << "vertexNormals: " << spot.vertexNormals.size() << std::endl;


    std::vector<Maths::Vec4f> clipPositions(spot.vertices.size());
    std::vector<Maths::Vec3f> worldNormals(spot.vertices.size());

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
        Maths::Vec3<>(0.0f, 0.0f, 0.0f),
    Maths::Vec3<>(3.14f, t, 0.0f),
    Maths::Vec3<>(1.0f, 1.0f, 1.0f)
);
        Maths::Matrix4x4<> view = Maths::LookAt(
            Maths::Vec3<>(0.0f, 0.0f, 0.4f),
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



        for (size_t i = 0; i < spot.vertices.size(); ++i)
        {
            clipPositions[i] = MVP * Maths::Vec4f(spot.vertices[i], 1.0f);
            worldNormals[i] = model.TransformDirection(spot.vertexNormals[i]);
        }

        for (auto& f : spot.faces)
        {
            ClipVertex v0 = { clipPositions[f.v.x], spot.uvs[f.uv.x], worldNormals[f.v.x] };
            ClipVertex v1 = { clipPositions[f.v.y], spot.uvs[f.uv.y], worldNormals[f.v.y] };
            ClipVertex v2 = { clipPositions[f.v.z], spot.uvs[f.uv.z], worldNormals[f.v.z] };


            ClipVertex clipped[6];
            int numTris = clipTriangleNear(v0, v1, v2, clipped);

            for (int i = 0; i < numTris; ++i)
            {
                const ClipVertex& tv0 = clipped[i * 3 + 0];
                const ClipVertex& tv1 = clipped[i * 3 + 1];
                const ClipVertex& tv2 = clipped[i * 3 + 2];

                // Division perspective (1 / w)
                Maths::Vec3f invW = { 1.0f / tv0.clip.w, 1.0f / tv1.clip.w, 1.0f / tv2.clip.w };

                // Coordonnées NDC
                Maths::Vec3f ndc0 = { tv0.clip.x * invW.x, tv0.clip.y * invW.x, tv0.clip.z * invW.x };
                Maths::Vec3f ndc1 = { tv1.clip.x * invW.y, tv1.clip.y * invW.y, tv1.clip.z * invW.y };
                Maths::Vec3f ndc2 = { tv2.clip.x * invW.z, tv2.clip.y * invW.z, tv2.clip.z * invW.z };

                // Transformation Viewport
                Maths::Vec2i sc0 = viewport(ndc0.x, ndc0.y, RENDER_WIDTH, RENDER_HEIGHT);
                Maths::Vec2i sc1 = viewport(ndc1.x, ndc1.y, RENDER_WIDTH, RENDER_HEIGHT);
                Maths::Vec2i sc2 = viewport(ndc2.x, ndc2.y, RENDER_WIDTH, RENDER_HEIGHT);

                Maths::Vec3f a = { (float)sc0.x, (float)sc0.y, ndc0.z };
                Maths::Vec3f b = { (float)sc1.x, (float)sc1.y, ndc1.z };
                Maths::Vec3f c = { (float)sc2.x, (float)sc2.y, ndc2.z };

                // Backface Culling en Screen Space
                int area = edge(a, b, c);
                if (area >= 0) continue;

                // Rasterisation
                drawTriangle(
                    a, b, c,
                    tv0.uv, tv1.uv, tv2.uv,
                    invW,
                    tv0.normal, tv1.normal, tv2.normal,
                    framebuffer, zbuffer, AssetTexture, light
                );
            }
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
