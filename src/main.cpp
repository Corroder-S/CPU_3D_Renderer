#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <cstdint>
#include <iostream>

#include "Maths/Matrix4x4.h"

constexpr int RENDER_WIDTH = 1280;
constexpr int RENDER_HEIGHT = 800;

struct Vec2 {
    int x;
    int y;
};

void put_pixel(std::vector<uint32_t>& framebuffer, int x, int y, uint32_t color)
{
    if (x < 0 || x >= RENDER_WIDTH || y < 0 || y >= RENDER_HEIGHT) return;
    framebuffer[y * RENDER_WIDTH + x] = color;
}

void draw_line(std::vector<uint32_t>& framebuffer, int x0, int x1, int y0, int y1, uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        put_pixel(framebuffer, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

uint32_t convert_color(uint8_t red, uint8_t green, uint8_t blue) {
    uint32_t color = ((uint32_t)0xFFu << 24) | (red << 16) | (green << 8) | (blue);
    return color;
}

uint32_t shade(
    int w0, int w1, int w2, int wsum,
    uint32_t c0, uint32_t c1, uint32_t c2)
{
    if (wsum == 0) return 0x000000FF;

    auto channel = [&](int shift) -> uint32_t {
        int64_t value =
            static_cast<int64_t>(w0) * ((c0 >> shift) & 0xFF) +
            static_cast<int64_t>(w1) * ((c1 >> shift) & 0xFF) +
            static_cast<int64_t>(w2) * ((c2 >> shift) & 0xFF);

        return static_cast<uint32_t>(value / wsum);
        };

    uint32_t r = channel(16);
    uint32_t g = channel(8);
    uint32_t b = channel(0);

    return (0xFFu << 24) | (r << 16) | (g << 8) | b;
}

int edge(Vec2 va, Vec2 vb, Vec2 vc) {
    return (vb.x - va.x) * (vc.y - va.y) - (vb.y - va.y) * (vc.x - va.x);
}

void draw_triangle(std::vector<uint32_t>& framebuffer, Vec2 v0, Vec2 v1, Vec2 v2, uint32_t c0, uint32_t c1, uint32_t c2)
{
    int area = edge(v0, v1, v2);
    if (area == 0) return;
    if (area < 0) {
        std::swap(v1, v2);
        std::swap(c1, c2);
        area = -area;
    }

    int minx = std::min(std::min(v0.x, v1.x), v2.x);
    int miny = std::min(std::min(v0.y, v1.y), v2.y);
    int maxx = std::max(std::max(v0.x, v1.x), v2.x);
    int maxy = std::max(std::max(v0.y, v1.y), v2.y);

    for (int py = miny; py < maxy; ++py){
        for (int px = minx; px < maxx; ++px) {
            int w0 = edge(v1, v2, Vec2(px, py));
            int w1 = edge(v2, v0, Vec2(px, py));
            int w2 = edge(v0, v1, Vec2(px, py));
            if (w0 > 0 && w1 > 0 && w2 > 0) {
                uint32_t color = shade(w0, w1, w2, w0 + w1 + w2, c0, c1, c2);
                put_pixel(framebuffer, px, py, color);
            }
        }
    }

}


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

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    bool isRunning = true;
    SDL_Event event;

    int8_t red, green, blue;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        std::fill(framebuffer.begin(), framebuffer.end(), 0x1A1A2EFF);

        //framebuffer[100 * RENDER_WIDTH + 160] = 0xFF0000FF;
        //framebuffer[50 * RENDER_WIDTH + 70] = 0xFF0000FF;
        //put_pixel(framebuffer, 50, 180, 0xFF0000FF);

       /*for (int y = 0; y < RENDER_HEIGHT; ++y) {
            for (int x = 0; x < RENDER_WIDTH; ++x) {
                red = static_cast<uint8_t>(x * 255 / RENDER_WIDTH);
                green = static_cast < uint8_t>(64);
                blue = static_cast<uint8_t>(y * 255 / RENDER_HEIGHT);
                put_pixel(framebuffer, x, y, convert_color(red, green, blue));
            }
       }*/
        Vec2 v0 = Vec2(400, 400);
        Vec2 v1 = Vec2(600, 600);
        Vec2 v2 = Vec2(200, 800);
        draw_triangle(
            framebuffer,
            v0, v1, v2,
            0xFFFF0000,  // rouge
            0xFF00FF00,  // vert
            0xFF0000FF   // bleu
        );


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