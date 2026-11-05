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

struct FragmentInput {
    Maths::Vec2f uv;
    Maths::Vec3f normal;
    Maths::Vec3f worldPos;
};

struct FragmentInput_8 {
    Maths::Vec2<__m256> uv;
    Maths::Vec3<__m256> normal;
    Maths::Vec3<__m256> worldPos;
    Color8 baseColor;
};

struct ShaderUniforms {
    const Material* material = nullptr;
    const Light* lights = nullptr;
    uint32_t lightCount = 0;
    Maths::Vec3f    cameraPos;
};

inline Color evaluateBlinnPhong(const FragmentInput& in, const ShaderUniforms& u)
{
    if (!u.material || !u.lights || u.lightCount == 0) return Color::White;

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

    Maths::Vec3f N = Maths::Normalize(in.normal);

    float metallic = std::clamp(u.material->metallic, 0.0f, 1.0f);

    // Diffuse & Ambiante
    Color diffColor = albedo * (1.0f - 0.5f * metallic);
    Color ambient = diffColor * (u.material->ambient * u.lights[0].ambient);

    // Si Ks est nul dans le MTL, blanc pur pour le diamant / albedo pour le m�tal
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

    Color finalColor = ambient + totalDiffuse + totalSpecular;

    finalColor.r = std::clamp(finalColor.r, 0.0f, 1.0f);
    finalColor.g = std::clamp(finalColor.g, 0.0f, 1.0f);
    finalColor.b = std::clamp(finalColor.b, 0.0f, 1.0f);

    float alpha = albedo.a * u.material->dissolve;
    finalColor.a = std::clamp(alpha, 0.0f, 1.0f);

    return finalColor;
}

inline Color8 unpackTexel8(__m256i texel)
{
    __m256 scale = _mm256_set1_ps(1.0f / 255.0f);
    auto ch = [&](__m256i c, int shift) -> __m256 {
        return _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(c, shift), _mm256_set1_epi32(0xFF))), scale);
    };
    return { ch(texel, 16), ch(texel, 8), ch(texel, 0), ch(texel, 24) };
}

inline __m256 computeSpecFactor8(
    const Maths::Vec3<__m256>& N, const Maths::Vec3<__m256>& L,
    const Maths::Vec3<__m256>& V, __m256 vMask, __m256 NdotL, float shininess)
{
    __m256 specMask = _mm256_and_ps(_mm256_cmp_ps(NdotL, _mm256_setzero_ps(), _CMP_GT_OS), vMask);
    Maths::Vec3<__m256> H = { _mm256_add_ps(L.x, V.x), _mm256_add_ps(L.y, V.y), _mm256_add_ps(L.z, V.z) };
    __m256 hMask = _mm256_cmp_ps(Maths::SIMD::Dot8(H, H), _mm256_setzero_ps(), _CMP_GT_OS);
    H = Maths::SIMD::Normalize8(H);
    specMask = _mm256_and_ps(specMask, hMask);

    __m256 NdotH    = _mm256_max_ps(_mm256_set1_ps(1e-8f), Maths::SIMD::Dot8(N, H));
    __m256 specFactor = exp2_ps(_mm256_mul_ps(_mm256_set1_ps(shininess), log2_ps(NdotH)));
    return _mm256_and_ps(specFactor, specMask);
}

inline Color8 evaluateBlinnPhong_8(const FragmentInput_8& in, const ShaderUniforms& u, Maths::Vec3<__m256> C)
{
    const __m256 zero = _mm256_setzero_ps();
    const __m256 one = _mm256_set1_ps(1.0f);

    if (!u.material || !u.lights || u.lightCount == 0)
        return { _mm256_set1_ps(1.f), _mm256_set1_ps(1.f), _mm256_set1_ps(1.f), _mm256_set1_ps(1.f) };

    float diffA = (u.material->diffuse.a <= 0.f) ? 1.f : u.material->diffuse.a;
    Color8 albedo = { _mm256_set1_ps(u.material->diffuse.r), _mm256_set1_ps(u.material->diffuse.g),
                      _mm256_set1_ps(u.material->diffuse.b), _mm256_set1_ps(diffA) };

    if (u.material->texture)
        albedo = albedo * unpackTexel8(u.material->texture->sampleBilinear_8(in.uv.x, in.uv.y));

    albedo = albedo * in.baseColor;

    albedo.r = _mm256_mul_ps(albedo.r, albedo.r);
    albedo.g = _mm256_mul_ps(albedo.g, albedo.g);
    albedo.b = _mm256_mul_ps(albedo.b, albedo.b);

    Maths::Vec3<__m256> N = Maths::SIMD::Normalize8(in.normal);
    Color  ambLight = u.material->ambient * u.lights[0].ambient;
    Color8 ambient  = albedo * Color8{ _mm256_set1_ps(ambLight.r), _mm256_set1_ps(ambLight.g), _mm256_set1_ps(ambLight.b), _mm256_set1_ps(ambLight.a) };

    Color8 totalDiffuse  = { _mm256_setzero_ps(), _mm256_setzero_ps(), _mm256_setzero_ps(), _mm256_setzero_ps() };
    Color8 totalSpecular = { _mm256_setzero_ps(), _mm256_setzero_ps(), _mm256_setzero_ps(), _mm256_setzero_ps() };

    bool hasSpecular = (u.material->shininess > 0.f) &&
        (u.material->specular.r > 0.f || u.material->specular.g > 0.f || u.material->specular.b > 0.f);

    Maths::Vec3<__m256> V;
    __m256 vMask = _mm256_setzero_ps();
    if (hasSpecular) {
        Maths::Vec3<__m256> dirV = { _mm256_sub_ps(C.x, in.worldPos.x), _mm256_sub_ps(C.y, in.worldPos.y), _mm256_sub_ps(C.z, in.worldPos.z) };
        vMask = _mm256_cmp_ps(Maths::SIMD::Dot8(dirV, dirV), _mm256_setzero_ps(), _CMP_GT_OS);
        V = Maths::SIMD::Normalize8(dirV);
    }

    for (uint32_t li = 0; li < u.lightCount; ++li) {
        Maths::Vec3<__m256> Lnorm = Maths::SIMD::Normalize8({
            _mm256_set1_ps(u.lights[li].direction.x),
            _mm256_set1_ps(u.lights[li].direction.y),
            _mm256_set1_ps(u.lights[li].direction.z) });

        __m256 NdotL = _mm256_max_ps(_mm256_setzero_ps(), Maths::SIMD::Dot8(N, Lnorm));
        totalDiffuse.r = _mm256_add_ps(totalDiffuse.r, _mm256_mul_ps(albedo.r, NdotL));
        totalDiffuse.g = _mm256_add_ps(totalDiffuse.g, _mm256_mul_ps(albedo.g, NdotL));
        totalDiffuse.b = _mm256_add_ps(totalDiffuse.b, _mm256_mul_ps(albedo.b, NdotL));

        if (hasSpecular) {
            __m256 sf = computeSpecFactor8(N, Lnorm, V, vMask, NdotL, u.material->shininess);
            totalSpecular.r = _mm256_add_ps(totalSpecular.r, _mm256_mul_ps(_mm256_set1_ps(u.material->specular.r), sf));
            totalSpecular.g = _mm256_add_ps(totalSpecular.g, _mm256_mul_ps(_mm256_set1_ps(u.material->specular.g), sf));
            totalSpecular.b = _mm256_add_ps(totalSpecular.b, _mm256_mul_ps(_mm256_set1_ps(u.material->specular.b), sf));
        }
    }

    __m256 one = _mm256_set1_ps(1.f), zero = _mm256_setzero_ps();
    Color8 finalColor;
    finalColor.r = _mm256_min_ps(_mm256_max_ps(_mm256_add_ps(_mm256_add_ps(ambient.r, totalDiffuse.r), totalSpecular.r), zero), one);
    finalColor.g = _mm256_min_ps(_mm256_max_ps(_mm256_add_ps(_mm256_add_ps(ambient.g, totalDiffuse.g), totalSpecular.g), zero), one);
    finalColor.b = _mm256_min_ps(_mm256_max_ps(_mm256_add_ps(_mm256_add_ps(ambient.b, totalDiffuse.b), totalSpecular.b), zero), one);
    finalColor.a = _mm256_min_ps(_mm256_max_ps(_mm256_mul_ps(albedo.a, _mm256_set1_ps(u.material->dissolve)), zero), one);

    finalColor.a = _mm256_min_ps(_mm256_max_ps(_mm256_mul_ps(albedo.a, _mm256_set1_ps(u.material->dissolve)), zero), one);
    return finalColor;
}