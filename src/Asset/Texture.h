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


    inline __m256i sampleBilinear_8(__m256 u, __m256 v) const {
        if (pixels.empty()) return _mm256_set1_epi32(0xFFFFFFFF);



        u = _mm256_and_ps(u, _mm256_cmp_ps(u, u, _CMP_ORD_Q));
        v = _mm256_and_ps(v, _mm256_cmp_ps(v, v, _CMP_ORD_Q));

        u = _mm256_sub_ps(u, _mm256_floor_ps(u));
        v = _mm256_sub_ps(v, _mm256_floor_ps(v));

        __m256 texX =  _mm256_mul_ps(u, _mm256_set1_ps(static_cast<float>(width - 1)));
        __m256 texY = _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0f), v), _mm256_set1_ps(static_cast<float>(height - 1)));

        __m256 ftexX = _mm256_floor_ps(texX);
        __m256 ftexY = _mm256_floor_ps(texY);

        __m256i x0 =  _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(ftexX, _mm256_setzero_ps()), _mm256_set1_ps(static_cast<float>(width - 1))));
        __m256i y0 =  _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(ftexY, _mm256_setzero_ps()), _mm256_set1_ps(static_cast<float>(height - 1))));

        __m256i x1 = _mm256_min_epi32(_mm256_add_epi32(x0, _mm256_set1_epi32(1)), _mm256_set1_epi32(width - 1));
        __m256i y1 = _mm256_min_epi32(_mm256_add_epi32(y0, _mm256_set1_epi32(1)), _mm256_set1_epi32(height - 1));

        __m256 fx = _mm256_sub_ps(texX, _mm256_cvtepi32_ps(x0));
        __m256 fy = _mm256_sub_ps(texY, _mm256_cvtepi32_ps(y0));

        __m256 w00 = _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0f), fx), _mm256_sub_ps(_mm256_set1_ps(1.0f), fy));
        __m256 w10 = _mm256_mul_ps(fx, _mm256_sub_ps(_mm256_set1_ps(1.0f), fy));
        __m256 w01 = _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1), fx), fy);
        __m256 w11 = _mm256_mul_ps(fx, fy);


        const uint32_t* texPixels = pixels.data();

        __m256i idx00 = _mm256_add_epi32(_mm256_mullo_epi32(y0, _mm256_set1_epi32(width)), x0);
        __m256i idx10 = _mm256_add_epi32(_mm256_mullo_epi32(y0, _mm256_set1_epi32(width)), x1);
        __m256i idx01 = _mm256_add_epi32(_mm256_mullo_epi32(y1, _mm256_set1_epi32(width)), x0);
        __m256i idx11 = _mm256_add_epi32(_mm256_mullo_epi32(y1, _mm256_set1_epi32(width)), x1);

        __m256i c00 = _mm256_i32gather_epi32((const int*)texPixels, idx00,4);
        __m256i c10 = _mm256_i32gather_epi32((const int*)texPixels, idx10,4);
        __m256i c01 = _mm256_i32gather_epi32((const int*)texPixels, idx01,4);
        __m256i c11 = _mm256_i32gather_epi32((const int*)texPixels, idx11,4);


        __m256i r =  ;

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



