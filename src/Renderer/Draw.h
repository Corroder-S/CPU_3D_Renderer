#pragma once

#include "Framebuffer.h"
#include "Maths/Vec2.h"
#include "Texture.h"
#include <cstdint>

static inline void drawline(int x0, int y0, int x1, int y1, uint32_t color, std::vector<uint32_t>& framebuffer)
{
    int dx =  abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;

    while (true)
    {
        put_pixel(x0, y0, color, framebuffer, RENDER_WIDTH, RENDER_HEIGHT);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static inline int edge(Maths::Vec3f a, Maths::Vec3f b, Maths::Vec3f c)
{
    return (int) (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}



static inline void drawTriangle(Maths::Vec3f a, Maths::Vec3f b, Maths::Vec3f c,
                                Maths::Vec2f uvx, Maths::Vec2f uvy, Maths::Vec2f uvz,
                                Maths::Vec3f invW,
                                std::vector<uint32_t>& framebuffer,
                                std::vector<float>& zbuffer,
                                const Texture& texture)
{
    int area = edge(a, b, c);
    if (area == 0) return;
    if (area < 0)
    {
        std::swap(b, c);
        std::swap(uvy, uvz);
        std::swap(invW.y, invW.z);
        area = -area;
    }

    int minX = std::max(0,                 std::min({(int)a.x, (int)b.x, (int)c.x}));
    int maxX = std::min(RENDER_WIDTH  - 1, std::max({(int)a.x, (int)b.x, (int)c.x}));
    int minY = std::max(0,                 std::min({(int)a.y, (int)b.y, (int)c.y}));
    int maxY = std::min(RENDER_HEIGHT - 1, std::max({(int)a.y, (int)b.y, (int)c.y}));

    float u0w = uvx.x * invW.x,  v0w = uvx.y * invW.x;
    float u1w = uvy.x * invW.y,  v1w = uvy.y * invW.y;
    float u2w = uvz.x * invW.z,  v2w = uvz.y * invW.z;


    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            if (x < 0 || x >= RENDER_WIDTH || y < 0 || y >= RENDER_HEIGHT) continue;


            Maths::Vec3f p = {(float)x, (float)y, 0.0f};
            int w0 = edge(b, c, p);
            int w1 = edge(c, a, p);
            int w2 = edge(a, b, p);
            if ((w0 | w1 | w2) >= 0)
            {
                float z = (w0 * a.z + w1 * b.z + w2 * c.z) / (float)area;
                if (z < zbuffer[y * RENDER_WIDTH + x])
                {
                    float uw = (w0 * u0w + w1 * u1w + w2 * u2w) / area;
                    float vw = (w0 * v0w + w1 * v1w + w2 * v2w) / area;
                    float iw = (w0 * invW.x + w1 * invW.y + w2 * invW.z) / area;
                    float u = uw / iw;
                    float v = vw / iw;


                    int tx = std::clamp((int)(u * texture.width),  0, texture.width  - 1);
                    int ty = std::clamp((int)((1.0f - v) * texture.height), 0, texture.height - 1);

                    uint32_t color = texture.pixels[ty * texture.width + tx];
                    zbuffer[y * RENDER_WIDTH + x] = z;
                    put_pixel(x, y, color, framebuffer, RENDER_WIDTH, RENDER_HEIGHT);
                }
            }
        }
    }
}
