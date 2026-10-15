#pragma once

#include <vector>
#include <cstdint>

constexpr int RENDER_WIDTH  = 900;
constexpr int RENDER_HEIGHT = 700;

static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)0xFFu << 24)
         | ((uint32_t)r    << 16)
         | ((uint32_t)g    <<  8)
         |  (uint32_t)b;
}

static inline void put_pixel(int x, int y, uint32_t color, std::vector<uint32_t>& framebuffer, int width, int height)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        return;
    framebuffer[y * width + x] = color;
}

static inline void clear(uint32_t color, std::vector<uint32_t>& framebuffer)
{
    for (int i = 0; i < RENDER_WIDTH * RENDER_HEIGHT; i++)
        framebuffer[i] = color;
}
