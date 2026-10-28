#pragma once

#include <cmath>
#include <algorithm>
#include "Maths/Vec2.h"
#include "Maths/Vec3.h"
#include "Asset/Color.h"
#include "Asset/Material.h"
#include "Asset/Light.h"
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
    const Light* light = nullptr;
    Maths::Vec3f    cameraPos;
};

inline Color evaluateBlinnPhong(const FragmentInput& in, const ShaderUniforms& u, Maths::Vec3f L)
{
    if (!u.material || !u.light) return Color::White;

    Color albedo = u.material->diffuse;

    if (u.material->texture) {
        uint32_t texel = u.material->texture->sampleBilinear(in.uv.x, in.uv.y);
        float ta = ((texel >> 24) & 0xFF) / 255.0f;
        float tr = ((texel >> 16) & 0xFF) / 255.0f;
        float tg = ((texel >> 8) & 0xFF) / 255.0f;
        float tb = (texel & 0xFF) / 255.0f;
        albedo = albedo * Color(tr, tg, tb, ta);
    }

    Maths::Vec3f N = Maths::Normalize(in.normal);

    Color ambient = albedo * (u.material->ambient * u.light->ambient);

    float NdotL = std::max(0.0f, Maths::Dot(N, L));
    Color diffuse = albedo * NdotL;

    Color specular;

    if (NdotL > 0.0f) {
        Maths::Vec3f dirV = u.cameraPos - in.worldPos;
        if (dirV.MagnitudeSquared() > 0.0f) {
            Maths::Vec3f V = Maths::Normalize(dirV);
            Maths::Vec3f H = L + V;
            if (H.MagnitudeSquared() > 0.0f) {
                H = Maths::Normalize(H);
                float NdotH = std::max(0.0f, Maths::Dot(N, H));
                float specFactor = std::pow(NdotH, u.material->shininess);
                specular = u.material->specular * specFactor;
            }
        }
    }

    Color finalColor = ambient + diffuse + specular;

    float alpha = albedo.a * u.material->dissolve;
    finalColor.a = std::clamp(alpha, 0.0f, 1.0f);

    return finalColor;
}



inline Color8 evaluateBlinnPhong_8(const FragmentInput_8& in, const ShaderUniforms& u, Maths::Vec3<__m256> L, Maths::Vec3<__m256> C)
{
    if (!u.material || !u.light)
    {
        return Color8{ _mm256_set1_ps(1.0f), _mm256_set1_ps(1.0f), _mm256_set1_ps(1.0f), _mm256_set1_ps(1.0f) };
    }


    Color8 albedo = { _mm256_set1_ps(u.material->diffuse.r), _mm256_set1_ps(u.material->diffuse.g), _mm256_set1_ps(u.material->diffuse.b), _mm256_set1_ps(u.material->diffuse.a) };


    if (u.material->texture) {
        __m256i texel = u.material->texture->sampleBilinear_8(in.uv.x, in.uv.y);

        __m256 a = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(texel, 24), _mm256_set1_epi32(0xFF)));
        __m256 r = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(texel, 16), _mm256_set1_epi32(0xFF)));
        __m256 g = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(texel, 8), _mm256_set1_epi32(0xFF)));
        __m256 b = _mm256_cvtepi32_ps(_mm256_and_si256(texel, _mm256_set1_epi32(0xFF)));

        __m256 ta = _mm256_mul_ps(a, _mm256_set1_ps(1.0f / 255.0f));
        __m256 tr = _mm256_mul_ps(r, _mm256_set1_ps(1.0f / 255.0f));
        __m256 tg = _mm256_mul_ps(g, _mm256_set1_ps(1.0f / 255.0f));
        __m256 tb = _mm256_mul_ps(b, _mm256_set1_ps(1.0f / 255.0f));

        albedo = albedo * Color8(tr, tg, tb, ta);
    }

    Maths::Vec3<__m256> N = Maths::SIMD::Normalize8(in.normal);




    Color ambLight = u.material->ambient * u.light->ambient;
    Color8 ambLight8 = { _mm256_set1_ps(ambLight.r), _mm256_set1_ps(ambLight.g), _mm256_set1_ps(ambLight.b), _mm256_set1_ps(ambLight.a) };
    Color8 ambient = albedo * ambLight8;

    __m256 NdotL = _mm256_max_ps(_mm256_setzero_ps(), Maths::SIMD::Dot8(N, L));

    Color8 diffuse = albedo * NdotL;

    Color8 specular;

    __m256 specMask = _mm256_cmp_ps(NdotL, _mm256_setzero_ps(), _CMP_GT_OS);

    Maths::Vec3<__m256> V = Maths::SIMD::Normalize8({_mm256_sub_ps( C.x,in.worldPos.x), _mm256_sub_ps(C.y, in.worldPos.y), _mm256_sub_ps(C.z, in.worldPos.z) });
    Maths::Vec3<__m256> H = Maths::SIMD::Normalize8({ _mm256_add_ps(L.x, V.x), _mm256_add_ps(L.y, V.y), _mm256_add_ps(L.z, V.z) });

    __m256 NdotH = _mm256_max_ps(_mm256_setzero_ps(), Maths::SIMD::Dot8(N, H));

    alignas(32) float ndoth[8], result[8];
    _mm256_store_ps(ndoth, NdotH);
    for (int i = 0; i < 8; i++)
    {
        result[i] = (ndoth[i] > 0.0f) ? std::exp2(u.material->shininess * std::log2(ndoth[i])) : 0.0f;
    }
    __m256 specFactor = _mm256_load_ps(result);

    specular = {_mm256_set1_ps(u.material->specular.r), _mm256_set1_ps(u.material->specular.g), _mm256_set1_ps(u.material->specular.b), _mm256_set1_ps(u.material->specular.a) };

    specular = specular * specFactor;

    specular.r = _mm256_and_ps(specular.r, specMask);
    specular.g = _mm256_and_ps(specular.g, specMask);
    specular.b = _mm256_and_ps(specular.b, specMask);

    Color8 finalColor = ambient + diffuse + specular;

    __m256 alpha = _mm256_mul_ps(albedo.a, _mm256_set1_ps(u.material->dissolve));
    finalColor.a = _mm256_min_ps(_mm256_max_ps(alpha, _mm256_setzero_ps()), _mm256_set1_ps(1.0f));

    return finalColor;
}