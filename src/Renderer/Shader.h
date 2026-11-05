#pragma once

#include <cmath>
#include <algorithm>
#include "Maths/Vec2.h"
#include "Maths/Vec3.h"
#include "Asset/Color.h"
#include "Asset/Material.h"
#include "Asset/Light.h"
#include "Maths/SIMD/Utils.h"
#include "Maths/SIMD/Vec3.h"
#include "Asset/Metal.h"

// ============================================================================
// SHADER PIPELINE STRUCTURES
// ============================================================================

/// @brief Données d'entrée scalaires d'un fragment pour le shading pixel.
struct FragmentInput {
    Maths::Vec2f uv;
    Maths::Vec3f normal;
    Maths::Vec3f worldPos;
};

/// @brief Données d'entrée SIMD (8 fragments traités en parallèle).
struct FragmentInput_8 {
    Maths::Vec2<__m256> uv;
    Maths::Vec3<__m256> normal;
    Maths::Vec3<__m256> worldPos;
    Color8 baseColor;
};

/// @brief Uniformes de rendu transmis au shader (Matériau, Lumières, Caméra).
struct ShaderUniforms {
    const Material* material = nullptr;
    const Light* lights = nullptr;
    uint32_t lightCount = 0;
    Maths::Vec3f cameraPos;
};

// ============================================================================
// SCALAR SHADING FUNCTIONS
// ============================================================================

/// @brief Évalue l'éclairage Blinn-Phong pour un fragment scalaire.
inline Color evaluateBlinnPhong(const FragmentInput& in, const ShaderUniforms& u)
{
    if (!u.material || !u.lights || u.lightCount == 0) return Color::White;

    // 1. Échantillonnage de l'Albedo
    Color albedo = u.material->diffuse;
    if (albedo.a <= 0.0f) albedo.a = 1.0f;

    if (u.material->texture) {
        uint32_t texel = u.material->texture->sampleBilinear(in.uv.x, in.uv.y);
        float ta = ((texel >> 24) & 0xFF) / 255.0f;
        float tr = ((texel >> 16) & 0xFF) / 255.0f;
        float tg = ((texel >> 8) & 0xFF) / 255.0f;
        float tb = (texel & 0xFF) / 255.0f;
        albedo = albedo * Color(tr, tg, tb, ta);
    }

    // 2. Préparation des normales et propriétés de surface
    Maths::Vec3f N = Maths::Normalize(in.normal);
    float metallic = std::clamp(u.material->metallic, 0.0f, 1.0f);

    Color diffColor = albedo * (1.0f - 0.5f * metallic);
    Color ambient = diffColor * (u.material->ambient * u.lights[0].ambient);

    float specSum = u.material->specular.r + u.material->specular.g + u.material->specular.b;
    Color baseSpec = (specSum > 0.05f) ? u.material->specular : Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color specColor = Color::lerp(baseSpec, albedo, metallic);

    Color totalDiffuse = Color(0.0f, 0.0f, 0.0f, 0.0f);
    Color totalSpecular = Color(0.0f, 0.0f, 0.0f, 0.0f);
    float shininess = (u.material->shininess > 0.0f) ? u.material->shininess : 32.0f;

    Maths::Vec3f V(0.0f, 0.0f, 0.0f);
    Maths::Vec3f dirV = u.cameraPos - in.worldPos;
    if (dirV.MagnitudeSquared() > 0.0f) {
        V = Maths::Normalize(dirV);
    }

    // 3. Accumulation de l'éclairage
    for (uint32_t li = 0; li < u.lightCount; ++li)
    {
        Maths::Vec3f Lnorm = Maths::Normalize(u.lights[li].direction);

        float NdotL = std::max(0.0f, Maths::Dot(N, Lnorm));
        totalDiffuse = totalDiffuse + diffColor * NdotL;

        if (NdotL > 0.0f)
        {
            Maths::Vec3f H = Lnorm + V;
            if (H.MagnitudeSquared() > 0.0f) {
                H = Maths::Normalize(H);
                float NdotH = std::max(0.0f, Maths::Dot(N, H));
                float specFactor = std::pow(NdotH, shininess);
                totalSpecular = totalSpecular + specColor * specFactor;
            }
        }
    }

    // 4. Couleur finale et découpage [0, 1]
    Color finalColor = ambient + totalDiffuse + totalSpecular;
    finalColor.r = std::clamp(finalColor.r, 0.0f, 1.0f);
    finalColor.g = std::clamp(finalColor.g, 0.0f, 1.0f);
    finalColor.b = std::clamp(finalColor.b, 0.0f, 1.0f);

    float alpha = albedo.a * u.material->dissolve;
    finalColor.a = std::clamp(alpha, 0.0f, 1.0f);

    return finalColor;
}

// ============================================================================
// SIMD VECTORIZED SHADING HELPERS
// ============================================================================

/// @brief Décompresse 8 texels 32-bit ARGB en une couleur SIMD 8 lanes.
inline Color8 unpackTexel8(__m256i texel)
{
    __m256 scale = _mm256_set1_ps(1.0f / 255.0f);
    auto ch = [&](__m256i c, int shift) -> __m256 {
        return _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(c, shift), _mm256_set1_epi32(0xFF))), scale);
    };
    return { ch(texel, 16), ch(texel, 8), ch(texel, 0), ch(texel, 24) };
}

/// @brief Évalue l'éclairage Blinn-Phong SIMD sur 8 pixels simultanément.
inline Color8 evaluateBlinnPhong_8(const FragmentInput_8& in, const ShaderUniforms& u, Maths::Vec3<__m256> C)
{
    const __m256 zero = _mm256_setzero_ps();
    const __m256 one = _mm256_set1_ps(1.0f);

    if (!u.material || !u.lights || u.lightCount == 0)
        return { one, one, one, one };

    // 1. Initialisation Albedo et modulation par Vertex Colors
    float diffA = (u.material->diffuse.a <= 0.0f) ? 1.0f : u.material->diffuse.a;
    Color8 albedo = { _mm256_set1_ps(u.material->diffuse.r), _mm256_set1_ps(u.material->diffuse.g),
                      _mm256_set1_ps(u.material->diffuse.b), _mm256_set1_ps(diffA) };

    if (u.material->texture)
        albedo = albedo * unpackTexel8(u.material->texture->sampleBilinear_8(in.uv.x, in.uv.y));

    albedo = albedo * in.baseColor;

    // Conversion en espace linéaire (Square)
    albedo.r = _mm256_mul_ps(albedo.r, albedo.r);
    albedo.g = _mm256_mul_ps(albedo.g, albedo.g);
    albedo.b = _mm256_mul_ps(albedo.b, albedo.b);

    // 2. Normalisation et Dispatch vers Shaders Spécialisés (PBR / Metal / Glass)
    Maths::Vec3<__m256> N = Maths::SIMD::Normalize8(in.normal);

    if (u.material->metallic > 0.0f)
        return Metal::shade_8(in.worldPos, N, C, albedo, *u.material, u.lights, u.lightCount);

    if (u.material->glass > 0.0f)
        return Metal::shadeGlass_8(in.worldPos, N, C, albedo, *u.material, u.lights, u.lightCount);

    // 3. Accumulation Ambiante, Diffuse et Spéculaire (Blinn-Phong)
    Color ambLight = u.material->ambient * u.lights[0].ambient;
    __m256 ambR = _mm256_mul_ps(albedo.r, _mm256_set1_ps(ambLight.r));
    __m256 ambG = _mm256_mul_ps(albedo.g, _mm256_set1_ps(ambLight.g));
    __m256 ambB = _mm256_mul_ps(albedo.b, _mm256_set1_ps(ambLight.b));

    __m256 diffAcc = zero;
    __m256 specAcc = zero;

    float specSum = u.material->specular.r + u.material->specular.g + u.material->specular.b;
    float sR = (specSum > 0.05f) ? u.material->specular.r : 1.0f;
    float sG = (specSum > 0.05f) ? u.material->specular.g : 1.0f;
    float sB = (specSum > 0.05f) ? u.material->specular.b : 1.0f;

    Maths::Vec3<__m256> dirV = { _mm256_sub_ps(C.x, in.worldPos.x),
                                 _mm256_sub_ps(C.y, in.worldPos.y),
                                 _mm256_sub_ps(C.z, in.worldPos.z) };
    __m256 vMask = _mm256_cmp_ps(Maths::SIMD::Dot8(dirV, dirV), _mm256_set1_ps(1e-8f), _CMP_GT_OS);
    Maths::Vec3<__m256> V = Maths::SIMD::Normalize8(dirV);

    float shinVal = (u.material->shininess > 0.0f) ? u.material->shininess : 32.0f;
    const __m256 shin = _mm256_set1_ps(shinVal);

    for (uint32_t li = 0; li < u.lightCount; ++li)
    {
        Maths::Vec3f Ld = Maths::Normalize(u.lights[li].direction);
        Maths::Vec3<__m256> L = { _mm256_set1_ps(Ld.x), _mm256_set1_ps(Ld.y), _mm256_set1_ps(Ld.z) };

        __m256 NdotL = _mm256_max_ps(zero, Maths::SIMD::Dot8(N, L));
        diffAcc = _mm256_add_ps(diffAcc, NdotL);

        Maths::Vec3<__m256> dirH = { _mm256_add_ps(L.x, V.x), _mm256_add_ps(L.y, V.y), _mm256_add_ps(L.z, V.z) };
        __m256 hMask = _mm256_cmp_ps(Maths::SIMD::Dot8(dirH, dirH), _mm256_set1_ps(1e-8f), _CMP_GT_OS);
        Maths::Vec3<__m256> H = Maths::SIMD::Normalize8(dirH);

        __m256 mask = _mm256_and_ps(_mm256_cmp_ps(NdotL, zero, _CMP_GT_OS), _mm256_and_ps(vMask, hMask));

        __m256 NdotH = _mm256_min_ps(one, _mm256_max_ps(_mm256_set1_ps(1e-4f), Maths::SIMD::Dot8(N, H)));
        __m256 specFactor = exp2_ps(_mm256_mul_ps(shin, log2_ps(NdotH)));
        specAcc = _mm256_add_ps(specAcc, _mm256_and_ps(specFactor, mask));
    }

    // 4. Assemblage final et correction Gamma (Sqrt vers espace sRGB)
    Color8 finalColor;
    finalColor.r = _mm256_add_ps(_mm256_add_ps(ambR, _mm256_mul_ps(albedo.r, diffAcc)), _mm256_mul_ps(_mm256_set1_ps(sR), specAcc));
    finalColor.g = _mm256_add_ps(_mm256_add_ps(ambG, _mm256_mul_ps(albedo.g, diffAcc)), _mm256_mul_ps(_mm256_set1_ps(sG), specAcc));
    finalColor.b = _mm256_add_ps(_mm256_add_ps(ambB, _mm256_mul_ps(albedo.b, diffAcc)), _mm256_mul_ps(_mm256_set1_ps(sB), specAcc));

    finalColor.r = _mm256_sqrt_ps(_mm256_min_ps(_mm256_max_ps(finalColor.r, zero), one));
    finalColor.g = _mm256_sqrt_ps(_mm256_min_ps(_mm256_max_ps(finalColor.g, zero), one));
    finalColor.b = _mm256_sqrt_ps(_mm256_min_ps(_mm256_max_ps(finalColor.b, zero), one));

    finalColor.a = _mm256_min_ps(_mm256_max_ps(_mm256_mul_ps(albedo.a, _mm256_set1_ps(u.material->dissolve)), zero), one);
    return finalColor;
}