#pragma once

#include <cmath>
#include <algorithm>
#include "Maths/Vec2.h"
#include "Maths/Vec3.h"
#include "src/Asset/Color.h"
#include "src/Asset/Material.h"
#include "src/Asset/Light.h"

struct FragmentInput {
    Maths::Vec2f uv;
    Maths::Vec3f normal;
    Maths::Vec3f worldPos;
};

struct ShaderUniforms {
    const Material* material = nullptr;
    const Light* light = nullptr;
    Maths::Vec3f    cameraPos;
};

inline Color evaluateBlinnPhong(const FragmentInput& in, const ShaderUniforms& u)
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
    Maths::Vec3f L = Maths::Normalize(u.light->direction);

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