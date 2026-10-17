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


