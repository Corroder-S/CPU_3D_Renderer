#pragma once
#include <cstdint>

namespace Maths
{
// ============================================================================
// UTILITAIRES MATHÉMATIQUES
// ============================================================================

/// @brief Interpolation linéaire générique entre deux valeurs.
/// @param a Valeur de départ.
/// @param b Valeur d'arrivée.
/// @param t Facteur ∈ [0, 1].
/// @return Valeur interpolée.
template< typename T >
inline float lerp(T a, T b, float t)
{
    return a + (b - a) * t;
}

/// @brief Interpolation linéaire entre deux couleurs ARGB 32 bits.
/// Chaque canal (A, R, G, B) est interpolé indépendamment.
/// @param a Couleur de départ (format 0xAARRGGBB).
/// @param b Couleur d'arrivée (format 0xAARRGGBB).
/// @param t Facteur ∈ [0, 1].
/// @return Couleur interpolée au format 0xAARRGGBB.
inline uint32_t lerpColor(uint32_t a, uint32_t b, float t)
{
    // 1. Décomposition des canaux de chaque couleur
    uint8_t a_r = (a >> 16) & 0xFF;
    uint8_t a_g = (a >>  8) & 0xFF;
    uint8_t a_b = (a >>  0) & 0xFF;
    uint8_t a_a = (a >> 24) & 0xFF;

    uint8_t b_r = (b >> 16) & 0xFF;
    uint8_t b_g = (b >>  8) & 0xFF;
    uint8_t b_b = (b >>  0) & 0xFF;
    uint8_t b_a = (b >> 24) & 0xFF;

    // 2. Interpolation canal par canal
    uint8_t r = static_cast<uint8_t>(a_r + (b_r - a_r) * t);
    uint8_t g = static_cast<uint8_t>(a_g + (b_g - a_g) * t);
    uint8_t bv = static_cast<uint8_t>(a_b + (b_b - a_b) * t);
    uint8_t av = static_cast<uint8_t>(a_a + (b_a - a_a) * t);

    // 3. Recomposition au format 0xAARRGGBB
    return (static_cast<uint32_t>(av) << 24) |
           (static_cast<uint32_t>(r)  << 16) |
           (static_cast<uint32_t>(g)  <<  8) |
           (static_cast<uint32_t>(bv));
}
}
