#pragma once

#include "Framebuffer.h"
#include "Maths/Vec2.h"
#include "../Asset/Texture.h"
#include <cstdint>

#include "Pipeline.h"
#include "Asset/Light.h"
#include "Maths/Matrix4x4.h"


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
    Maths::Vec3f wn0, Maths::Vec3f wn1, Maths::Vec3f wn2,
    std::vector<uint32_t>& framebuffer,
    std::vector<float>& zbuffer,
    const Texture& texture,
    const Light& light)
{
    int area = edge(a, b, c);
    if (area == 0) return;
    if (area < 0)
    {
        std::swap(b, c);
        std::swap(uvy, uvz);
        std::swap(invW.y, invW.z);
        std::swap(wn1, wn2);
        area = -area;
    }

    int minX = std::max(0, std::min({ (int)a.x, (int)b.x, (int)c.x }));
    int maxX = std::min(RENDER_WIDTH - 1, std::max({ (int)a.x, (int)b.x, (int)c.x }));
    int minY = std::max(0, std::min({ (int)a.y, (int)b.y, (int)c.y }));
    int maxY = std::min(RENDER_HEIGHT - 1, std::max({ (int)a.y, (int)b.y, (int)c.y }));

    float invArea = 1.0f / static_cast<float>(area);
    float u0w = uvx.x * invW.x, v0w = uvx.y * invW.x;
    float u1w = uvy.x * invW.y, v1w = uvy.y * invW.y;
    float u2w = uvz.x * invW.z, v2w = uvz.y * invW.z;

    uint32_t* fb = framebuffer.data();
    float* zb = zbuffer.data();

    for (int y = minY; y <= maxY; y++)
    {
        int rowOffset = y * RENDER_WIDTH;
        for (int x = minX; x <= maxX; x++)
        {
            Maths::Vec3f p = { static_cast<float>(x), static_cast<float>(y), 0.0f };
            int w0 = edge(b, c, p);
            int w1 = edge(c, a, p);
            int w2 = edge(a, b, p);

            if ((w0 | w1 | w2) >= 0)
            {
                float z = (w0 * a.z + w1 * b.z + w2 * c.z) * invArea;
                int idx = rowOffset + x;

                if (z >= zb[idx]) continue;

                float fw0 = w0 * invArea;
                float fw1 = w1 * invArea;
                float fw2 = w2 * invArea;

                float iw = fw0 * invW.x + fw1 * invW.y + fw2 * invW.z;
                float invIw = 1.0f / iw;

                float u = (fw0 * u0w + fw1 * u1w + fw2 * u2w) * invIw;
                float v = (fw0 * v0w + fw1 * v1w + fw2 * v2w) * invIw;

                Maths::Vec3f N = Maths::Normalize(wn0 * fw0 + wn1 * fw1 + wn2 * fw2);

                float brightness;
                computeBrightness(N, light.direction, light.ambient, brightness);

                uint32_t texel = texture.sampleBilinear(u, v);

                uint8_t r = static_cast<uint8_t>(((texel >> 16) & 0xFF) * brightness);
                uint8_t g = static_cast<uint8_t>(((texel >> 8) & 0xFF) * brightness);
                uint8_t b = static_cast<uint8_t>((texel & 0xFF) * brightness);
                uint32_t color = (texel & 0xFF000000) | (r << 16) | (g << 8) | b;

                zb[idx] = z;
                fb[idx] = color;
            }
        }
    }
}