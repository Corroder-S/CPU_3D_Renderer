#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"


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

    std::cout << "[Texture] Charg�e : " << filepath << " (" << width << "x" << height << ")" << std::endl;
    return true;
}
