#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <cstdint>
#include <iostream>

#include "Maths/Matrix4x4.h"

constexpr int RENDER_WIDTH = 1280;
constexpr int RENDER_HEIGHT = 800;

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

static Maths::Matrix4x4<> Perspective(float FOV, float aspect, float near, float far)
{
    Maths::Matrix4x4<> result;
    result = result.Zero();
    float yscale = 1 / (tan(FOV / 2));
    float xscale = yscale / aspect;
    float A = (far + near) / (near - far);
    float B = (2 * far * near) / (near - far);

    result.values[0][0] = xscale;
    result.values[1][1] = yscale;
    result.values[2][2] = A;
    result.values[2][3] = B;
    result.values[3][2] = -1;

    return result;
}

static Maths::Matrix4x4<> LookAt(const Maths::Vec3<>& eye, const Maths::Vec3<>& target, const Maths::Vec3<>& up)
{   
    Maths::Vec3<> forward = Maths::Normalize(eye - target);
    Maths::Vec3<> right = Maths::Normalize(Maths::Cross(up, forward));
    Maths::Vec3<> camUp = Maths::Cross(forward, right);

    Maths::Matrix4x4<> result;
    result = result.Identity();

    result.values[0][0] = right.x;
    result.values[0][1] = right.y;
    result.values[0][2] = right.z;
    result.values[0][3] = -Maths::Dot(right, eye);
    result.values[1][0] = camUp.x; 
    result.values[1][1] = camUp.y;
    result.values[1][2] = camUp.z;  
    result.values[1][3] = -Maths::Dot(camUp, eye);
    result.values[2][0] = forward.x;
    result.values[2][1] = forward.y;
    result.values[2][2] = forward.z;
    result.values[2][3] = -Maths::Dot(forward, eye);

    return result;
}

bool project_vertex(const Maths::Vec3<float>& vertex,const Maths::Matrix4x4<float>& mvp,int width, int height, Maths::Vec3<float>& out_pixel)
{
    Maths::Vec4<float> clip = mvp * Maths::Vec4<float>(vertex, 1.0f);
    if (clip.w <= 0.001f) return false;

    Maths::Vec3<float> ndc(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w);

    out_pixel.x = (ndc.x + 1.0f) * 0.5f * static_cast<float>(width);
    out_pixel.y = (1.0f - (ndc.y + 1.0f) * 0.5f) * static_cast<float>(height);
    out_pixel.z = ndc.z;

    return true;
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

int edge(Maths::Vec3<> va, Maths::Vec3<> vb, Maths::Vec3<> vc) {
    return (vb.x - va.x) * (vc.y - va.y) - (vb.y - va.y) * (vc.x - va.x);
}

void draw_triangle(std::vector<uint32_t>& framebuffer, Maths::Vec3<> v0, Maths::Vec3<> v1, Maths::Vec3<> v2, uint32_t c0, uint32_t c1, uint32_t c2)
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
            int w0 = edge(v1, v2, Maths::Vec3<>(float(px), float(py), 0));
            int w1 = edge(v2, v0, Maths::Vec3<>(float(px), float(py), 0));
            int w2 = edge(v0, v1, Maths::Vec3<>(float(px), float(py), 0));
            if (w0 > 0 && w1 > 0 && w2 > 0) {
                uint32_t color = shade(w0, w1, w2, w0 + w1 + w2, c0, c1, c2);
                put_pixel(framebuffer, px, py, color);
            }
        }
    }

}


Maths::Vec3<> rotate_vec(Maths::Vec3<>& v, float angle)
{
    return Maths::Matrix4x4<float>::RotationZ(angle).TransformPoint(v);
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

    Maths::Vec3<> v0 = Maths::Vec3<>(400, 400, 0);
    Maths::Vec3<> v1 = Maths::Vec3<>(600, 600, 0);
    Maths::Vec3<> v2 = Maths::Vec3<>(200, 800, 0);

    Maths::Vec3<> v0_base = Maths::Vec3<>(400, 400, 0);
    Maths::Vec3<> v1_base = Maths::Vec3<>(600, 600, 0);
    Maths::Vec3<> v2_base = Maths::Vec3<>(200, 800, 0);


    const std::vector<Maths::Vec3<float>> cube_vertices = {
    {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}};

    const std::vector<std::pair<int, int>> cube_edges = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    Maths::Vec3<> TriangleCenter(
        (v0_base.x + v1_base.x + v2_base.x) / 3.0f,
        (v0_base.y + v1_base.y + v2_base.y) / 3.0f,
        (v0_base.z + v1_base.z + v2_base.z) / 3.0f
    );


    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }


        float t = SDL_GetTicks() / 1000.f;

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

        /*draw_triangle(
            framebuffer,
            v0, v1, v2,
            0xFFFF0000,  // rouge
            0xFF00FF00,  // vert
            0xFF0000FF   // bleu
        );



        

        v0 = M.TransformPoint(v0_base);
        v1 = M.TransformPoint(v1_base);
        v2 = M.TransformPoint(v2_base);*/

        Maths::Matrix4x4<float> model = Maths::Matrix4x4<float>::RotationY(t) * Maths::Matrix4x4<float>::RotationX(t * 0.5f);
        Maths::Matrix4x4<float> view = LookAt({ 0.0f, 0.0f, 3.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
        Maths::Matrix4x4<float> proj = Perspective(1.0472f, static_cast<float>(RENDER_WIDTH) / RENDER_HEIGHT, 0.1f, 100.0f);

        Maths::Matrix4x4<float> mvp = proj * view * model;

        std::vector<Maths::Vec3<float>> projected_pixels(cube_vertices.size());
        std::vector<bool> valid_vertices(cube_vertices.size());

        for (size_t i = 0; i < cube_vertices.size(); ++i) {
            valid_vertices[i] = project_vertex(cube_vertices[i], mvp, RENDER_WIDTH, RENDER_HEIGHT, projected_pixels[i]);
        }

        for (const auto& edge : cube_edges) {
            int i0 = edge.first;
            int i1 = edge.second;

            if (valid_vertices[i0] && valid_vertices[i1]) {
                draw_line(
                    framebuffer,
                    static_cast<int>(projected_pixels[i0].x),
                    static_cast<int>(projected_pixels[i1].x),
                    static_cast<int>(projected_pixels[i0].y),
                    static_cast<int>(projected_pixels[i1].y),
                    0xFFFFFFFF
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