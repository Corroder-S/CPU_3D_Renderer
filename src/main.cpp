#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <cstdint>
#include <iostream>

constexpr int RENDER_WIDTH = 320;
constexpr int RENDER_HEIGHT = 200;

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Erreur SDL_Init : " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "3D CPU Engine",
        1280, 800,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "Erreur création fenêtre : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH, RENDER_HEIGHT
    );

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    std::vector<uint32_t> framebuffer(RENDER_WIDTH * RENDER_HEIGHT);

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        std::fill(framebuffer.begin(), framebuffer.end(), 0x1A1A2EFF);

        framebuffer[100 * RENDER_WIDTH + 160] = 0xFF0000FF;
        framebuffer[50 * RENDER_WIDTH + 70] = 0xFF0000FF;

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