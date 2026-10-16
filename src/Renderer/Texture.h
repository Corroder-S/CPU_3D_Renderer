#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include "External/stb/stb_image.h"

struct Texture {
    int width = 0;
    int height = 0;
    std::vector<uint32_t> pixels;

    bool load(const std::string& filepath);

    bool isValid() const {
        return !pixels.empty() && width > 0 && height > 0;
    }
};


bool Texture::load(const std::string& filepath) {
    int channels;

    stbi_uc* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);

    if (!data) {
        std::cerr << "[Texture] Erreur de chargement : " << filepath
            << " (" << stbi_failure_reason() << ")" << std::endl;
        return false;
    }

    pixels.resize(width * height);

    for (int i = 0; i < width * height; ++i) {
        uint8_t r = data[i * 4 + 0];
        uint8_t g = data[i * 4 + 1];
        uint8_t b = data[i * 4 + 2];
        uint8_t a = data[i * 4 + 3];

        pixels[i] = (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(r) << 16) |
            (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(b);
    }

    stbi_image_free(data);

    std::cout << "[Texture] Chargée : " << filepath << " (" << width << "x" << height << ")" << std::endl;
    return true;
}
