#pragma once

#include <vector>
#include <numeric>
#include <random>
#include <cmath>
#include <algorithm>

// ============================================================================
// NOISE GENERATOR (PERLIN 2D & FRACTALS)
// ============================================================================

/// @brief Générateur de bruit de Perlin 2D et d'algorithmes fractals (FBM, Ridged, Domain Warp).
class PerlinNoise {
private:
    std::vector<int> p; // Table de permutation répétée (512 éléments)

    /// @brief Courbe d'atténuation quintique de Perlin (6t^5 - 15t^4 + 10t^3).
    static float fade(float t) {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    /// @brief Interpole linéairement deux valeurs scalaires.
    static float lerp(float t, float a, float b) {
        return a + t * (b - a);
    }

    /// @brief Calcule le produit scalaire entre le vecteur gradient de la grille et le vecteur de distance.
    static float grad(int hash, float x, float y) {
        int h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }

public:
    /// @brief Initialise la table de permutation du bruit à partir d'une graine (seed).
    /// @param seed Graine aléatoire pour la génération.
    explicit PerlinNoise(int seed) {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);
        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);
        p.insert(p.end(), p.begin(), p.end());
    }

    /// @brief Échantillonne une valeur de bruit de Perlin 2D standard.
    /// @param x Coordonnée X.
    /// @param y Coordonnée Y.
    /// @return Valeur de bruit continue.
    float noise(float x, float y) const {
        // 1. Détermination des coordonnées de la cellule de grille (unit cube)
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;

        // 2. Coordonnées relatives à l'intérieur de la cellule [0, 1]
        x -= std::floor(x);
        y -= std::floor(y);

        // 3. Calcul des facteurs d'atténuation (fade curves)
        float u = fade(x);
        float v = fade(y);

        // 4. Hachage des coins de la cellule
        int A = p[X] + Y;
        int B = p[X + 1] + Y;

        // 5. Interpolation bilinéaire des 4 gradients
        return lerp(v, lerp(u, grad(p[A], x, y), grad(p[B], x - 1.0f, y)),
            lerp(u, grad(p[A + 1], x, y - 1.0f), grad(p[B + 1], x - 1.0f, y - 1.0f)));
    }

    /// @brief Génère un bruit fractal FBM (Fractional Brownian Motion) par superposition d'octaves.
    /// @param octaves Nombre de couches de détails à cumuler.
    /// @param persistence Facteur d'atténuation de l'amplitude à chaque octave.
    /// @param lacunarity Multiplicateur de fréquence à chaque octave.
    float fbm(float x, float y, int octaves, float persistence, float lacunarity) const {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; ++i) {
            total += noise(x * frequency, y * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return total / maxValue;
    }

    /// @brief Génère un bruit fractal Ridged (crêtes aiguës idéal pour les reliefs montagneux).
    float ridgedFbm(float x, float y, int octaves, float persistence, float lacunarity) const {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float weight = 1.0f;

        for (int i = 0; i < octaves; ++i) {
            float signal = noise(x * frequency, y * frequency);
            signal = std::abs(signal);
            signal = 1.0f - signal;
            signal *= signal;
            signal *= weight;
            weight = std::clamp(signal * 2.0f, 0.0f, 1.0f);

            total += signal * amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }
        return total;
    }

    /// @brief Génère un bruit FBM déformé par domaine (Domain Warping) pour des formes organiques.
    /// @param warpStrength Intensité de la déformation des coordonnées d'échantillonnage.
    float domainWarpFbm(float x, float y, int octaves, float persistence, float lacunarity, float warpStrength = 1.5f) const {
        float qx = fbm(x, y, octaves, persistence, lacunarity);
        float qz = fbm(x + 5.2f, y + 1.3f, octaves, persistence, lacunarity);
        return fbm(x + warpStrength * qx, y + warpStrength * qz, octaves, persistence, lacunarity);
    }
};