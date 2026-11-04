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

struct FragmentInput {
    Maths::Vec2f uv;
    Maths::Vec3f normal;
    Maths::Vec3f worldPos;
};

struct FragmentInput_8 {
    Maths::Vec2<__m256> uv;
    Maths::Vec3<__m256> normal;
    Maths::Vec3<__m256> worldPos;
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

    Color ambient = albedo * (u.material->ambient * u.lights[0].ambient);

    Color totalDiffuse = Color(0.0f, 0.0f, 0.0f, 0.0f);
    Color totalSpecular = Color(0.0f, 0.0f, 0.0f, 0.0f);

    bool hasSpecular = (u.material->shininess > 0.0f) &&
        (u.material->specular.r > 0.0f || u.material->specular.g > 0.0f || u.material->specular.b > 0.0f);

    Maths::Vec3f V;
    bool validV = false;
    if (hasSpecular) {
        Maths::Vec3f dirV = u.cameraPos - in.worldPos;
        if (dirV.MagnitudeSquared() > 0.0f) {
            V = Maths::Normalize(dirV);
            validV = true;
        }
    }

    for (uint32_t li = 0; li < u.lightCount; ++li)
    {
        Maths::Vec3f Lnorm = Maths::Normalize(u.lights[li].direction);

        float NdotL = std::max(0.0f, Maths::Dot(N, Lnorm));
        totalDiffuse = totalDiffuse + albedo * NdotL;

        if (hasSpecular && validV && NdotL > 0.0f)
        {
            Maths::Vec3f H = Lnorm + V;
            if (H.MagnitudeSquared() > 0.0f) {
                H = Maths::Normalize(H);
                float NdotH = std::max(0.0f, Maths::Dot(N, H));
                float specFactor = std::pow(NdotH, u.material->shininess);
                totalSpecular = totalSpecular + u.material->specular * specFactor;
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

inline Color8 evaluateBlinnPhong_8(const FragmentInput_8& in, const ShaderUniforms& u, Maths::Vec3<__m256> C)
{
    const __m256 zero = _mm256_setzero_ps();
    const __m256 one = _mm256_set1_ps(1.0f);

    if (!u.material || !u.lights || u.lightCount == 0)
        return Color8{ one, one, one, one };

    float diffA = (u.material->diffuse.a <= 0.0f) ? 1.0f : u.material->diffuse.a;
    Color8 albedo = {
        _mm256_set1_ps(u.material->diffuse.r),
        _mm256_set1_ps(u.material->diffuse.g),
        _mm256_set1_ps(u.material->diffuse.b),
        _mm256_set1_ps(diffA)
    };

    if (u.material->texture) {
        __m256i texel = u.material->texture->sampleBilinear_8(in.uv.x, in.uv.y);
        const __m256i m255 = _mm256_set1_epi32(0xFF);
        const __m256  inv255 = _mm256_set1_ps(1.0f / 255.0f);

        __m256 ta = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(texel, 24), m255)), inv255);
        __m256 tr = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(texel, 16), m255)), inv255);
        __m256 tg = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(texel, 8), m255)), inv255);
        __m256 tb = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(texel, m255)), inv255);

        albedo = albedo * Color8(tr, tg, tb, ta);
    }

    albedo.r = _mm256_mul_ps(albedo.r, albedo.r);
    albedo.g = _mm256_mul_ps(albedo.g, albedo.g);
    albedo.b = _mm256_mul_ps(albedo.b, albedo.b);

    Maths::Vec3<__m256> N = Maths::SIMD::Normalize8(in.normal);

    Color ambLight = u.material->ambient * u.lights[0].ambient;
    __m256 ambR = _mm256_mul_ps(albedo.r, _mm256_set1_ps(ambLight.r));
    __m256 ambG = _mm256_mul_ps(albedo.g, _mm256_set1_ps(ambLight.g));
    __m256 ambB = _mm256_mul_ps(albedo.b, _mm256_set1_ps(ambLight.b));

    __m256 diffAcc = zero;
    __m256 specAcc = zero;

    const bool hasSpecular = (u.material->shininess > 0.0f) &&
        (u.material->specular.r > 0.0f || u.material->specular.g > 0.0f || u.material->specular.b > 0.0f);

    Maths::Vec3<__m256> V{};
    __m256 vMask = zero;
    if (hasSpecular) {
        Maths::Vec3<__m256> dirV = { _mm256_sub_ps(C.x, in.worldPos.x),
                                     _mm256_sub_ps(C.y, in.worldPos.y),
                                     _mm256_sub_ps(C.z, in.worldPos.z) };
        vMask = _mm256_cmp_ps(Maths::SIMD::Dot8(dirV, dirV), _mm256_set1_ps(1e-8f), _CMP_GT_OS);
        V = Maths::SIMD::Normalize8(dirV);
    }

    const __m256 shin = _mm256_set1_ps(u.material->shininess);

    for (uint32_t li = 0; li < u.lightCount; ++li)
    {
        Maths::Vec3f Ld = Maths::Normalize(u.lights[li].direction);
        Maths::Vec3<__m256> L = { _mm256_set1_ps(Ld.x), _mm256_set1_ps(Ld.y), _mm256_set1_ps(Ld.z) };

        __m256 NdotL = _mm256_max_ps(zero, Maths::SIMD::Dot8(N, L));
        diffAcc = _mm256_add_ps(diffAcc, NdotL);

        if (hasSpecular)
        {
            Maths::Vec3<__m256> dirH = { _mm256_add_ps(L.x, V.x), _mm256_add_ps(L.y, V.y), _mm256_add_ps(L.z, V.z) };
            __m256 hMask = _mm256_cmp_ps(Maths::SIMD::Dot8(dirH, dirH), _mm256_set1_ps(1e-8f), _CMP_GT_OS);
            Maths::Vec3<__m256> H = Maths::SIMD::Normalize8(dirH);

            __m256 mask = _mm256_and_ps(_mm256_cmp_ps(NdotL, zero, _CMP_GT_OS),
                _mm256_and_ps(vMask, hMask));

            __m256 NdotH = _mm256_min_ps(one, _mm256_max_ps(_mm256_set1_ps(1e-4f), Maths::SIMD::Dot8(N, H)));

            __m256 specFactor = exp2_ps(_mm256_mul_ps(shin, log2_ps(NdotH)));
            specAcc = _mm256_add_ps(specAcc, _mm256_and_ps(specFactor, mask));
        }
    }

    Color8 finalColor;
    finalColor.r = _mm256_add_ps(_mm256_add_ps(ambR, _mm256_mul_ps(albedo.r, diffAcc)),
        _mm256_mul_ps(_mm256_set1_ps(u.material->specular.r), specAcc));
    finalColor.g = _mm256_add_ps(_mm256_add_ps(ambG, _mm256_mul_ps(albedo.g, diffAcc)),
        _mm256_mul_ps(_mm256_set1_ps(u.material->specular.g), specAcc));
    finalColor.b = _mm256_add_ps(_mm256_add_ps(ambB, _mm256_mul_ps(albedo.b, diffAcc)),
        _mm256_mul_ps(_mm256_set1_ps(u.material->specular.b), specAcc));

    finalColor.r = _mm256_min_ps(_mm256_max_ps(finalColor.r, zero), one);
    finalColor.g = _mm256_min_ps(_mm256_max_ps(finalColor.g, zero), one);
    finalColor.b = _mm256_min_ps(_mm256_max_ps(finalColor.b, zero), one);

    finalColor.r = _mm256_sqrt_ps(finalColor.r);
    finalColor.g = _mm256_sqrt_ps(finalColor.g);
    finalColor.b = _mm256_sqrt_ps(finalColor.b);

    finalColor.a = _mm256_min_ps(_mm256_max_ps(_mm256_mul_ps(albedo.a, _mm256_set1_ps(u.material->dissolve)), zero), one);
    return finalColor;
}