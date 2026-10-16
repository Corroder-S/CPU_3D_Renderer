#pragma once

#include "Framebuffer.h"
#include "Maths/Vec2.h"
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
                                uint32_t c0, uint32_t c1, uint32_t c2,
                                
                                std::vector<uint32_t>& framebuffer,
                                std::vector<float>& zbuffer)
{
    int area = edge(a, b, c);
    if (area == 0) return;
    if (area < 0)
    {
        std::swap(b, c);
        std::swap(c1, c2);
        area = -area;
    }

    int minX = std::max(0,                 std::min({(int)a.x, (int)b.x, (int)c.x}));
    int maxX = std::min(RENDER_WIDTH  - 1, std::max({(int)a.x, (int)b.x, (int)c.x}));
    int minY = std::max(0,                 std::min({(int)a.y, (int)b.y, (int)c.y}));
    int maxY = std::min(RENDER_HEIGHT - 1, std::max({(int)a.y, (int)b.y, (int)c.y}));

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
                   // int r  = (w0 * (int)((c0 >> 16) & 0xFF) + w1 * (int)((c1 >> 16) & 0xFF) + w2 * (int)((c2 >> 16) & 0xFF)) / area;
                   // int g  = (w0 * (int)((c0 >>  8) & 0xFF) + w1 * (int)((c1 >>  8) & 0xFF) + w2 * (int)((c2 >>  8) & 0xFF)) / area;
                   // int bl = (w0 * (int)( c0         & 0xFF) + w1 * (int)( c1        & 0xFF) + w2 * (int)( c2        & 0xFF)) / area;
                    zbuffer[y * RENDER_WIDTH + x] = z;
                    put_pixel(x, y, rgb(255, 255, 255), framebuffer, RENDER_WIDTH, RENDER_HEIGHT);
                }
            }
        }
    }
}
