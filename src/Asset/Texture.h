#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <immintrin.h>
#include "External/stb/stb_image.h"

// ============================================================================
// HELPER FUNCTIONS FOR SIMD TEXTURE SAMPLING
// ============================================================================

static constexpr float inv255 = 1.0f / 255.0f;

/// @brief Melange un canal de couleur (R, G, B ou A) pour 8 texels bilinéaires SIMD.
static inline __m256i bilinearBlendChannel(
    __m256i c00, __m256i c10, __m256i c01, __m256i c11,
    __m256 w00, __m256 w10, __m256 w01, __m256 w11, int shift)
{
    auto ch = [shift](__m256i c) -> __m256 {
        return _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(c, shift), _mm256_set1_epi32(0xFF)));
    };
    return _mm256_cvtps_epi32(_mm256_add_ps(
        _mm256_add_ps(_mm256_mul_ps(ch(c00), w00), _mm256_mul_ps(ch(c10), w10)),
        _mm256_add_ps(_mm256_mul_ps(ch(c01), w01), _mm256_mul_ps(ch(c11), w11))
    ));
}

// ============================================================================
// TEXTURE CLASS DEFINITION
// ============================================================================

/// @brief Gère le chargement et l'échantillonnage bilinéaire (Scalaire & SIMD AVX2) des images 2D.
struct Texture {
    int width = 0;
    int height = 0;
    std::vector<uint32_t> pixels; // Buffer ARGB 32-bit (format non-multiplié)

    /// @brief Charge une image depuis un fichier (PNG, JPG, BMP, etc.).
    bool load(const std::string& filepath);

    /// @brief Indique si la texture contient des pixels valides.
    bool isValid() const {
        return !pixels.empty() && width > 0 && height > 0;
    }

    /// @brief Échantillonne la texture de manière bilinéaire pour une coordonnée UV scalaire.
    inline uint32_t sampleBilinear(float u, float v) const {
        if (pixels.empty()) return 0xFFFFFFFF;

        // 1. Protection NaN et répétition UV [0, 1)
        if (!std::isfinite(u)) u = 0.0f;
        if (!std::isfinite(v)) v = 0.0f;

        u = u - std::floor(u);
        v = v - std::floor(v);

        // 2. Conversion UV vers coordonnées texels
        float texX = u * (width - 1);
        float texY = (1.0f - v) * (height - 1);

        int x0 = std::clamp(static_cast<int>(std::floor(texX)), 0, width - 1);
        int y0 = std::clamp(static_cast<int>(std::floor(texY)), 0, height - 1);
        int x1 = std::min(x0 + 1, width - 1);
        int y1 = std::min(y0 + 1, height - 1);

        // 3. Calcul des poids d'interpolation bilinéaire
        float fx = texX - x0;
        float fy = texY - y0;

        float w00 = (1.0f - fx) * (1.0f - fy);
        float w10 = fx * (1.0f - fy);
        float w01 = (1.0f - fx) * fy;
        float w11 = fx * fy;

        // 4. Lecture des 4 texels voisins
        const uint32_t* texPixels = pixels.data();
        uint32_t c00 = texPixels[y0 * width + x0];
        uint32_t c10 = texPixels[y0 * width + x1];
        uint32_t c01 = texPixels[y1 * width + x0];
        uint32_t c11 = texPixels[y1 * width + x1];

        // 5. Blend des canaux ARGB et re-conditionnement 32-bit
        uint8_t r = static_cast<uint8_t>(((c00 >> 16) & 0xFF) * w00 + ((c10 >> 16) & 0xFF) * w10 + ((c01 >> 16) & 0xFF) * w01 + ((c11 >> 16) & 0xFF) * w11);
        uint8_t g = static_cast<uint8_t>(((c00 >> 8) & 0xFF) * w00 + ((c10 >> 8) & 0xFF) * w10 + ((c01 >> 8) & 0xFF) * w01 + ((c11 >> 8) & 0xFF) * w11);
        uint8_t b = static_cast<uint8_t>((c00 & 0xFF) * w00 + (c10 & 0xFF) * w10 + (c01 & 0xFF) * w01 + (c11 & 0xFF) * w11);
        uint8_t a = static_cast<uint8_t>(((c00 >> 24) & 0xFF) * w00 + ((c10 >> 24) & 0xFF) * w10 + ((c01 >> 24) & 0xFF) * w01 + ((c11 >> 24) & 0xFF) * w11);

        return (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(r) << 16) |
            (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(b);
    }

    /// @brief Échantillonne la texture de manière bilinéaire pour 8 coordonnées UV en parallèle (AVX2).
    inline __m256i sampleBilinear_8(__m256 u, __m256 v) const {
        if (pixels.empty()) return _mm256_set1_epi32(0xFFFFFFFF);

        // 1. Guard NaN + répétition UV [0, 1) sur 8 lanes
        u = _mm256_and_ps(u, _mm256_cmp_ps(u, u, _CMP_ORD_Q));
        v = _mm256_and_ps(v, _mm256_cmp_ps(v, v, _CMP_ORD_Q));
        u = _mm256_sub_ps(u, _mm256_floor_ps(u));
        v = _mm256_sub_ps(v, _mm256_floor_ps(v));

        // 2. Conversion UV -> coordonnées texels
        __m256 texX = _mm256_mul_ps(u, _mm256_set1_ps((float)(width - 1)));
        __m256 texY = _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.f), v), _mm256_set1_ps((float)(height - 1)));

        // 3. Clamping des indices des 4 voisins aux limites de la texture
        __m256i x0 = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(_mm256_floor_ps(texX), _mm256_setzero_ps()), _mm256_set1_ps((float)(width - 1))));
        __m256i y0 = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(_mm256_floor_ps(texY), _mm256_setzero_ps()), _mm256_set1_ps((float)(height - 1))));
        __m256i x1 = _mm256_min_epi32(_mm256_add_epi32(x0, _mm256_set1_epi32(1)), _mm256_set1_epi32(width - 1));
        __m256i y1 = _mm256_min_epi32(_mm256_add_epi32(y0, _mm256_set1_epi32(1)), _mm256_set1_epi32(height - 1));

        // 4. Calcul des 4 poids bilinéaires SIMD
        __m256 fx = _mm256_sub_ps(texX, _mm256_cvtepi32_ps(x0));
        __m256 fy = _mm256_sub_ps(texY, _mm256_cvtepi32_ps(y0));
        __m256 ifx = _mm256_sub_ps(_mm256_set1_ps(1.f), fx);
        __m256 ify = _mm256_sub_ps(_mm256_set1_ps(1.f), fy);
        __m256 w00 = _mm256_mul_ps(ifx, ify), w10 = _mm256_mul_ps(fx, ify);
        __m256 w01 = _mm256_mul_ps(ifx, fy), w11 = _mm256_mul_ps(fx, fy);

        // 5. Gather SIMD des 4 texels voisins pour les 8 pixels simultanément
        const int* tex = (const int*)pixels.data();
        __m256i     stride = _mm256_set1_epi32(width);
        __m256i c00 = _mm256_i32gather_epi32(tex, _mm256_add_epi32(_mm256_mullo_epi32(y0, stride), x0), 4);
        __m256i c10 = _mm256_i32gather_epi32(tex, _mm256_add_epi32(_mm256_mullo_epi32(y0, stride), x1), 4);
        __m256i c01 = _mm256_i32gather_epi32(tex, _mm256_add_epi32(_mm256_mullo_epi32(y1, stride), x0), 4);
        __m256i c11 = _mm256_i32gather_epi32(tex, _mm256_add_epi32(_mm256_mullo_epi32(y1, stride), x1), 4);

        // 6. Blend individuel de chaque canal et réassemblage au format 8x32bit ARGB packed
        __m256i r = bilinearBlendChannel(c00, c10, c01, c11, w00, w10, w01, w11, 16);
        __m256i g = bilinearBlendChannel(c00, c10, c01, c11, w00, w10, w01, w11, 8);
        __m256i b = bilinearBlendChannel(c00, c10, c01, c11, w00, w10, w01, w11, 0);
        __m256i a = bilinearBlendChannel(c00, c10, c01, c11, w00, w10, w01, w11, 24);

        return _mm256_or_si256(
            _mm256_or_si256(_mm256_slli_epi32(a, 24), _mm256_slli_epi32(r, 16)),
            _mm256_or_si256(_mm256_slli_epi32(g, 8), b)
        );
    }
};