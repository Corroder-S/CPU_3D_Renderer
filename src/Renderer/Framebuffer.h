#pragma once

#include <vector>
#include <cstdint>
#include "Engine/Config.h"

// ============================================================================
// FRAMEBUFFER — UTILITAIRES DE PIXELS
// ============================================================================

/// @brief Construit une couleur ARGB opaque (alpha = 0xFF) à partir de composantes RGB.
/// @param r Canal rouge [0, 255].
/// @param g Canal vert [0, 255].
/// @param b Canal bleu [0, 255].
/// @return Couleur encodée au format ARGB 32 bits.
static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)0xFFu << 24)
         | ((uint32_t)r    << 16)
         | ((uint32_t)g    <<  8)
         |  (uint32_t)b;
}

/// @brief Écrit un pixel dans le framebuffer après vérification des bornes.
/// @param x       Coordonnée horizontale en pixels.
/// @param y       Coordonnée verticale en pixels.
/// @param color   Couleur ARGB 32 bits à écrire.
/// @param framebuffer Buffer destination.
/// @param width   Largeur du framebuffer en pixels.
/// @param height  Hauteur du framebuffer en pixels.
static inline void putPixel(int x, int y, uint32_t color, std::vector<uint32_t>& framebuffer, int width, int height)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        return;
    framebuffer[y * width + x] = color;
}

/// @brief Remplit l'intégralité du framebuffer avec une couleur uniforme.
/// @param color       Couleur ARGB 32 bits de remplissage.
/// @param framebuffer Buffer à effacer.
static inline void clear(uint32_t color, std::vector<uint32_t>& framebuffer)
{
    for (int i = 0; i < framebuffer.size(); i++)
        framebuffer[i] = color;
}
