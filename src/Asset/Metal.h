#pragma once

#include <cmath>
#include <algorithm>
#include "Maths/Vec3.h"
#include "Asset/Color.h"
#include "Asset/Material.h"
#include "Asset/Light.h"
#include "Maths/SIMD/Utils.h"
#include "Maths/SIMD/Vec3.h"

// ============================================================================
// SIMD PBR SHADING PIPELINE (METAL & GLASS)
// ============================================================================

namespace Metal {

    // --- Constantes Fisheye / Environment Shading ---
    constexpr float kLightDirSign = -1.0f;
    constexpr float kHorizonBlurMin = 0.30f;
    constexpr float kMetalGrazingBoost = 0.30f;
    constexpr float kGlassF0 = 0.04f;
    constexpr float kGlassBodyOpacity = 0.35f;
    constexpr float kGlassEnvScale = 0.60f;
    constexpr float kGlassDebugAlpha = 0.8f;
    constexpr int   kGlassDebugView = 0;
    constexpr float kGlassMaxShininess = 200.0f;
    constexpr float kSunColor[3] = { 1.00f, 0.95f, 0.85f };

    /// @brief Paramètres d'environnement global pour la carte de réflexion procédurale.
    struct Environment {
        float horizonSky[3] = { 0.20f, 0.22f, 0.30f };
        float zenith[3] = { 0.03f, 0.03f, 0.07f };
        float horizonGround[3] = { 0.12f, 0.30f, 0.12f };
        float nadir[3] = { 0.03f, 0.09f, 0.04f };
        float exposure = 1.0f;
        float saturation = 0.60f;
    };
    inline Environment g_env;

    /// @brief Reconfigure les couleurs d'ambiance de l'environnement sky/ground.
    inline void setEnvironment(const Color& sky, const Color& ground,
        float horizonLift = 0.15f, float exposure = 1.0f, float saturation = 0.60f)
    {
        const float s[3] = { sky.r * sky.r,       sky.g * sky.g,       sky.b * sky.b };
        const float g[3] = { ground.r * ground.r, ground.g * ground.g, ground.b * ground.b };
        for (int i = 0; i < 3; ++i) {
            g_env.zenith[i] = s[i];
            g_env.horizonSky[i] = s[i] + (1.0f - s[i]) * horizonLift;
            g_env.horizonGround[i] = g[i];
            g_env.nadir[i] = g[i] * 0.3f;
        }
        g_env.exposure = exposure;
        g_env.saturation = saturation;
    }

    /// @brief Utilitaires de conversion de couleur binaire 32-bit.
    inline Color colorFromARGB(uint32_t c)
    {
        return Color(((c >> 16) & 0xFF) / 255.0f, ((c >> 8) & 0xFF) / 255.0f, (c & 0xFF) / 255.0f, 1.0f);
    }

    // ============================================================================
    // INTERNAL SIMD MATH & HELPER FUNCTIONS
    // ============================================================================

    namespace detail {

        inline float envColor(const float* c, int i)
        {
            float grey = 0.2126f * c[0] + 0.7152f * c[1] + 0.0722f * c[2];
            return grey + (c[i] - grey) * g_env.saturation;
        }

        inline __m256 lerp8(__m256 a, __m256 b, __m256 t)
        {
            return _mm256_add_ps(a, _mm256_mul_ps(_mm256_sub_ps(b, a), t));
        }

        inline __m256 saturate8(__m256 x)
        {
            return _mm256_min_ps(_mm256_max_ps(x, _mm256_setzero_ps()), _mm256_set1_ps(1.0f));
        }

        inline __m256 smoothstep8(__m256 e0, __m256 e1, __m256 x)
        {
            __m256 t = saturate8(_mm256_div_ps(_mm256_sub_ps(x, e0), _mm256_sub_ps(e1, e0)));
            return _mm256_mul_ps(_mm256_mul_ps(t, t),
                _mm256_sub_ps(_mm256_set1_ps(3.0f), _mm256_mul_ps(_mm256_set1_ps(2.0f), t)));
        }

        inline __m256 pow5_8(__m256 x)
        {
            __m256 x2 = _mm256_mul_ps(x, x);
            return _mm256_mul_ps(_mm256_mul_ps(x2, x2), x);
        }

        inline __m256 pow8_8(__m256 x)
        {
            __m256 x2 = _mm256_mul_ps(x, x);
            __m256 x4 = _mm256_mul_ps(x2, x2);
            return _mm256_mul_ps(x4, x4);
        }

        inline Maths::Vec3f toLight(const Light& l)
        {
            Maths::Vec3f d = Maths::Normalize(l.direction);
            return Maths::Vec3f{ d.x * kLightDirSign, d.y * kLightDirSign, d.z * kLightDirSign };
        }

        struct Rgb8 { __m256 r, g, b; };

        /// @brief Échantillonne la carte de réflexion d'environnement pour 8 rayons SIMD.
        inline Rgb8 sampleEnvironment(const Maths::Vec3<__m256>& R, const Maths::Vec3f& toSun,
            float sunPower, float sunIntensity, float blur)
        {
            const __m256 zero = _mm256_setzero_ps();
            const __m256 expo = _mm256_set1_ps(g_env.exposure);

            __m256 up = _mm256_sqrt_ps(saturate8(R.y));
            __m256 down = _mm256_sqrt_ps(saturate8(_mm256_sub_ps(zero, R.y)));
            __m256 blend = smoothstep8(_mm256_set1_ps(-blur), _mm256_set1_ps(blur), R.y);

            __m256 d = _mm256_add_ps(_mm256_add_ps(
                _mm256_mul_ps(R.x, _mm256_set1_ps(toSun.x)),
                _mm256_mul_ps(R.y, _mm256_set1_ps(toSun.y))),
                _mm256_mul_ps(R.z, _mm256_set1_ps(toSun.z)));
            d = _mm256_max_ps(d, _mm256_set1_ps(1e-4f));
            __m256 e = _mm256_max_ps(_mm256_mul_ps(_mm256_set1_ps(sunPower), log2_ps(d)), _mm256_set1_ps(-126.0f));
            __m256 sun = _mm256_mul_ps(exp2_ps(e), _mm256_set1_ps(sunIntensity));

            auto channel = [&](int c) {
                __m256 sky = lerp8(_mm256_set1_ps(envColor(g_env.horizonSky, c)), _mm256_set1_ps(envColor(g_env.zenith, c)), up);
                __m256 ground = lerp8(_mm256_set1_ps(envColor(g_env.horizonGround, c)), _mm256_set1_ps(envColor(g_env.nadir, c)), down);
                __m256 env = _mm256_mul_ps(lerp8(ground, sky, blend), expo);
                return _mm256_add_ps(env, _mm256_mul_ps(sun, _mm256_set1_ps(envColor(kSunColor, c))));
                };
            return Rgb8{ channel(0), channel(1), channel(2) };
        }

        struct View8 {
            __m256 NdotV;
            Maths::Vec3<__m256> R;
        };

        /// @brief Calcule le produit scalaire NdotV et le rayon de réflexion R pour 8 pixels SIMD.
        inline View8 viewSetup(const Maths::Vec3<__m256>& worldPos, Maths::Vec3<__m256> N,
            const Maths::Vec3<__m256>& C)
        {
            const __m256 zero = _mm256_setzero_ps();

            Maths::Vec3<__m256> dirV = { _mm256_sub_ps(C.x, worldPos.x),
                                         _mm256_sub_ps(C.y, worldPos.y),
                                         _mm256_sub_ps(C.z, worldPos.z) };
            Maths::Vec3<__m256> V = Maths::SIMD::Normalize8(dirV);

            __m256 NdotVraw = Maths::SIMD::Dot8(N, V);

            // Inversion de la normale si l'observateur regarde la face arrière (back-face)
            __m256 backMask = _mm256_cmp_ps(NdotVraw, zero, _CMP_LT_OS);
            N.x = _mm256_blendv_ps(N.x, _mm256_sub_ps(zero, N.x), backMask);
            N.y = _mm256_blendv_ps(N.y, _mm256_sub_ps(zero, N.y), backMask);
            N.z = _mm256_blendv_ps(N.z, _mm256_sub_ps(zero, N.z), backMask);

            NdotVraw = _mm256_max_ps(zero, Maths::SIMD::Dot8(N, V));

            __m256 twoNV = _mm256_add_ps(NdotVraw, NdotVraw);
            View8 v;
            v.NdotV = saturate8(NdotVraw);

            v.R = { _mm256_sub_ps(_mm256_mul_ps(twoNV, N.x), V.x),
                    _mm256_sub_ps(_mm256_mul_ps(twoNV, N.y), V.y),
                    _mm256_sub_ps(_mm256_mul_ps(twoNV, N.z), V.z) };
            return v;
        }

        inline __m256 diffuseSum(const Maths::Vec3<__m256>& N, const Light* lights, uint32_t lightCount)
        {
            __m256 acc = _mm256_setzero_ps();
            for (uint32_t li = 0; li < lightCount; ++li)
            {
                Maths::Vec3f Ld = toLight(lights[li]);
                Maths::Vec3<__m256> L = { _mm256_set1_ps(Ld.x), _mm256_set1_ps(Ld.y), _mm256_set1_ps(Ld.z) };
                acc = _mm256_add_ps(acc, _mm256_max_ps(_mm256_setzero_ps(), Maths::SIMD::Dot8(N, L)));
            }
            return acc;
        }

    }

    // ============================================================================
    // SHADER IMPLEMENTATIONS (METALLIC & GLASS)
    // ============================================================================

    /// @brief Rendu PBR Métallique SIMD (Fresnel Schlick + Réflexion d'environnement).
    inline Color8 shade_8(const Maths::Vec3<__m256>& worldPos,
        const Maths::Vec3<__m256>& N,
        const Maths::Vec3<__m256>& C,
        const Color8& albedo,
        const Material& mat,
        const Light* lights, uint32_t lightCount)
    {
        using namespace detail;
        const __m256 one = _mm256_set1_ps(1.0f);

        // 1. Dérivation des paramètres d'état du matériau
        const float metallic = std::clamp(mat.metallic, 0.0f, 1.0f);
        const float shininess = std::max(mat.shininess, 1.0f);
        const float rough = std::sqrt(2.0f / (shininess + 2.0f));
        const float blur = std::min(1.0f, kHorizonBlurMin + 0.50f * rough);
        const float sunI = std::min(4.0f, (shininess + 2.0f) * 0.0796f);
        const Maths::Vec3f toSun = toLight(lights[0]);

        View8 v = viewSetup(worldPos, N, C);

        // 2. Approximation de Fresnel-Schlick pour matériaux métalliques/diélectriques
        const __m256 mm = _mm256_set1_ps(metallic);
        const __m256 diel = _mm256_set1_ps(0.04f);
        const __m256 smooth = _mm256_set1_ps(1.0f - rough);
        const __m256 fScale = _mm256_set1_ps(1.0f - metallic * (1.0f - kMetalGrazingBoost));
        __m256 f5 = _mm256_mul_ps(pow5_8(_mm256_sub_ps(one, v.NdotV)), fScale);

        __m256 F0r = lerp8(diel, albedo.r, mm);
        __m256 F0g = lerp8(diel, albedo.g, mm);
        __m256 F0b = lerp8(diel, albedo.b, mm);
        __m256 Fr = _mm256_add_ps(F0r, _mm256_mul_ps(_mm256_sub_ps(_mm256_max_ps(smooth, F0r), F0r), f5));
        __m256 Fg = _mm256_add_ps(F0g, _mm256_mul_ps(_mm256_sub_ps(_mm256_max_ps(smooth, F0g), F0g), f5));
        __m256 Fb = _mm256_add_ps(F0b, _mm256_mul_ps(_mm256_sub_ps(_mm256_max_ps(smooth, F0b), F0b), f5));

        // 3. Échantillonnage de la carte d'environnement
        Rgb8 env = sampleEnvironment(v.R, toSun, shininess, sunI, blur);

        // 4. Accumulation de la contribution diffuse
        __m256 diffAcc = diffuseSum(N, lights, lightCount);
        Color ambLight = mat.ambient * lights[0].ambient;
        const __m256 kd = _mm256_set1_ps(1.0f - metallic);

        __m256 lr = _mm256_mul_ps(_mm256_mul_ps(kd, albedo.r), _mm256_add_ps(diffAcc, _mm256_set1_ps(ambLight.r)));
        __m256 lg = _mm256_mul_ps(_mm256_mul_ps(kd, albedo.g), _mm256_add_ps(diffAcc, _mm256_set1_ps(ambLight.g)));
        __m256 lb = _mm256_mul_ps(_mm256_mul_ps(kd, albedo.b), _mm256_add_ps(diffAcc, _mm256_set1_ps(ambLight.b)));

        // 5. Assemblage final et correction Gamma (Sqrt)
        Color8 out;
        out.r = _mm256_sqrt_ps(saturate8(_mm256_add_ps(lr, _mm256_mul_ps(Fr, env.r))));
        out.g = _mm256_sqrt_ps(saturate8(_mm256_add_ps(lg, _mm256_mul_ps(Fg, env.g))));
        out.b = _mm256_sqrt_ps(saturate8(_mm256_add_ps(lb, _mm256_mul_ps(Fb, env.b))));
        out.a = saturate8(_mm256_mul_ps(albedo.a, _mm256_set1_ps(mat.dissolve)));
        return out;
    }

    /// @brief Rendu Transparent/Verre SIMD (Fresnel + Transmission/Réflexion combinées).
    inline Color8 shadeGlass_8(const Maths::Vec3<__m256>& worldPos,
        const Maths::Vec3<__m256>& N,
        const Maths::Vec3<__m256>& C,
        const Color8& albedo,
        const Material& mat,
        const Light* lights, uint32_t lightCount)
    {
        using namespace detail;
        const __m256 one = _mm256_set1_ps(1.0f);

        // 1. Préparation des paramètres physiques du verre
        const float shininess = std::clamp(mat.shininess, 1.0f, kGlassMaxShininess);
        const float rough = std::sqrt(2.0f / (shininess + 2.0f));
        const float blur = std::min(1.0f, kHorizonBlurMin + 0.50f * rough);
        const float sunI = std::min(4.0f, (shininess + 2.0f) * 0.0796f);
        const Maths::Vec3f toSun = toLight(lights[0]);

        View8 v = viewSetup(worldPos, N, C);

        // 2. Calcul du terme de Fresnel pour le verre (F0 = 0.04)
        const __m256 F0 = _mm256_set1_ps(kGlassF0);
        __m256 F = _mm256_add_ps(F0, _mm256_mul_ps(_mm256_sub_ps(one, F0), pow8_8(_mm256_sub_ps(one, v.NdotV))));

        // 3. Réflexion d'environnement pondérée
        Rgb8 env = sampleEnvironment(v.R, toSun, shininess, sunI, blur);
        const __m256 envScale = _mm256_set1_ps(kGlassEnvScale);
        env.r = _mm256_mul_ps(env.r, envScale);
        env.g = _mm256_mul_ps(env.g, envScale);
        env.b = _mm256_mul_ps(env.b, envScale);

        // 4. Transmission du corps et éclairage sous-jacent
        __m256 diffAcc = diffuseSum(N, lights, lightCount);
        Color ambLight = mat.ambient * lights[0].ambient;
        __m256 br = _mm256_mul_ps(albedo.r, _mm256_add_ps(diffAcc, _mm256_set1_ps(ambLight.r)));
        __m256 bg = _mm256_mul_ps(albedo.g, _mm256_add_ps(diffAcc, _mm256_set1_ps(ambLight.g)));
        __m256 bb = _mm256_mul_ps(albedo.b, _mm256_add_ps(diffAcc, _mm256_set1_ps(ambLight.b)));

        __m256 ab = saturate8(_mm256_mul_ps(albedo.a, _mm256_set1_ps(mat.dissolve * kGlassBodyOpacity)));
        __m256 a = _mm256_max_ps(_mm256_sub_ps(_mm256_add_ps(ab, F), _mm256_mul_ps(ab, F)), _mm256_set1_ps(1e-3f));

        // 5. Normalisation et sortie des canaux avec correction Gamma
        Color8 out;
        out.r = _mm256_sqrt_ps(saturate8(_mm256_div_ps(_mm256_add_ps(_mm256_mul_ps(br, ab), _mm256_mul_ps(env.r, F)), a)));
        out.g = _mm256_sqrt_ps(saturate8(_mm256_div_ps(_mm256_add_ps(_mm256_mul_ps(bg, ab), _mm256_mul_ps(env.g, F)), a)));
        out.b = _mm256_sqrt_ps(saturate8(_mm256_div_ps(_mm256_add_ps(_mm256_mul_ps(bb, ab), _mm256_mul_ps(env.b, F)), a)));
        out.a = (kGlassDebugAlpha >= 0.0f) ? _mm256_set1_ps(kGlassDebugAlpha) : a;
        if (kGlassDebugView == 1) {
            out.r = saturate8(F);
            out.g = saturate8(ab);
            out.b = v.NdotV;
            out.a = one;
        }
        return out;
    }

}