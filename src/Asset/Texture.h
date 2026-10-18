#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include "External/stb/stb_image.h"


struct Texture {
    int width = 0;
    int height = 0;
    std::vector<uint32_t> pixels;

    bool load(const std::string& filepath);

    bool isValid() const {
        return !pixels.empty() && width > 0 && height > 0;
    }

    inline uint32_t sampleBilinear(float u, float v) const {
        if (pixels.empty()) return 0xFFFFFFFF;

        if (!std::isfinite(u)) u = 0.0f;
        if (!std::isfinite(v)) v = 0.0f;

        u = u - std::floor(u);
        v = v - std::floor(v);

        float texX = u * (width - 1);
        float texY = (1.0f - v) * (height - 1);

        int x0 = std::clamp(static_cast<int>(std::floor(texX)), 0, width - 1);
        int y0 = std::clamp(static_cast<int>(std::floor(texY)), 0, height - 1);
        int x1 = std::min(x0 + 1, width - 1);
        int y1 = std::min(y0 + 1, height - 1);

        float fx = texX - x0;
        float fy = texY - y0;

        float w00 = (1.0f - fx) * (1.0f - fy);
        float w10 = fx * (1.0f - fy);
        float w01 = (1.0f - fx) * fy;
        float w11 = fx * fy;

        // Utilisation du pointeur brut direct .data()
        const uint32_t* texPixels = pixels.data();
        uint32_t c00 = texPixels[y0 * width + x0];
        uint32_t c10 = texPixels[y0 * width + x1];
        uint32_t c01 = texPixels[y1 * width + x0];
        uint32_t c11 = texPixels[y1 * width + x1];

        uint8_t r = static_cast<uint8_t>(((c00 >> 16) & 0xFF) * w00 + ((c10 >> 16) & 0xFF) * w10 + ((c01 >> 16) & 0xFF) * w01 + ((c11 >> 16) & 0xFF) * w11);
        uint8_t g = static_cast<uint8_t>(((c00 >> 8) & 0xFF) * w00 + ((c10 >> 8) & 0xFF) * w10 + ((c01 >> 8) & 0xFF) * w01 + ((c11 >> 8) & 0xFF) * w11);
        uint8_t b = static_cast<uint8_t>((c00 & 0xFF) * w00 + (c10 & 0xFF) * w10 + (c01 & 0xFF) * w01 + (c11 & 0xFF) * w11);
        uint8_t a = static_cast<uint8_t>(((c00 >> 24) & 0xFF) * w00 + ((c10 >> 24) & 0xFF) * w10 + ((c01 >> 24) & 0xFF) * w01 + ((c11 >> 24) & 0xFF) * w11);

        return (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(r) << 16) |
            (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(b);
    }
};



