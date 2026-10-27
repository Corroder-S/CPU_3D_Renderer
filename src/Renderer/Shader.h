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

    Maths::Vec3f N = Maths::SIMD::Normalize(in.normal);

    Color ambient = albedo * (u.material->ambient * u.light->ambient);

    float NdotL = std::max(0.0f, Maths::SIMD::Dot(N, L));
    Color diffuse = albedo * NdotL;

    Color specular;

    if (NdotL > 0.0f) {
        Maths::Vec3f V = Maths::SIMD::Normalize(u.cameraPos - in.worldPos);
        Maths::Vec3f H = Maths::SIMD::Normalize(L + V);
        float NdotH = std::max(0.0f, Maths::SIMD::Dot(N, H));
        float specFactor = (NdotH > 0.0f) ? std::exp2(u.material->shininess * std::log2(NdotH)) : 0.0f;
        specular = u.material->specular * specFactor;
    }

    Color finalColor = ambient + diffuse + specular;

    float alpha = albedo.a * u.material->dissolve;
    finalColor.a = std::clamp(alpha, 0.0f, 1.0f);

    return finalColor;
}


inline Color8 evaluateBlinnPhong_8(const FragmentInput& in, const ShaderUniforms& u, Maths::Vec3f L)
{
    if (!u.material || !u.light)
    {
        return Color8{ _mm256_set1_ps(1.0f), _mm256_set1_ps(1.0f), _mm256_set1_ps(1.0f), _mm256_set1_ps(1.0f) };
    }



    __m256 ambient_r = _mm256_set1_ps(u.material->ambient.r);
    __m256 ambient_g = _mm256_set1_ps(u.material->ambient.g);
    __m256 ambient_b = _mm256_set1_ps(u.material->ambient.b);
    __m256 ambient_a = _mm256_set1_ps(u.material->ambient.a);

    Color8 albedo{ ambient_r, ambient_g, ambient_b, ambient_a };



    if (u.material->texture) {
        uint32_t texel = u.material->texture->sampleBilinear(in.uv.x, in.uv.y);
        float ta = ((texel >> 24) & 0xFF) / 255.0f;
        float tr = ((texel >> 16) & 0xFF) / 255.0f;
        float tg = ((texel >> 8) & 0xFF) / 255.0f;
        float tb = (texel & 0xFF) / 255.0f;
        albedo = albedo * Color(tr, tg, tb, ta);
    }

    Maths::Vec3f N = Maths::SIMD::Normalize(in.normal);

    Color ambient = albedo * (u.material->ambient * u.light->ambient);

    float NdotL = std::max(0.0f, Maths::SIMD::Dot(N, L));
    Color diffuse = albedo * NdotL;

    Color specular;

    if (NdotL > 0.0f) {
        Maths::Vec3f V = Maths::SIMD::Normalize(u.cameraPos - in.worldPos);
        Maths::Vec3f H = Maths::SIMD::Normalize(L + V);
        float NdotH = std::max(0.0f, Maths::SIMD::Dot(N, H));
        float specFactor = (NdotH > 0.0f) ? std::exp2(u.material->shininess * std::log2(NdotH)) : 0.0f;
        specular = u.material->specular * specFactor;
    }

    Color finalColor = ambient + diffuse + specular;

    float alpha = albedo.a * u.material->dissolve;
    finalColor.a = std::clamp(alpha, 0.0f, 1.0f);

    return finalColor;
}