#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include <memory>

#include "Color.h"
#include "Texture.h"

// ============================================================================
// MATERIAL DEFINITION
// ============================================================================

/// @brief Structure décrivant les propriétés optiques et de surface d'un matériau.
struct Material {
    std::string name = "Default";

    // --- Propriétés d'éclairage classique (Phong / Blinn-Phong) ---
    Color ambient = Color::White;
    Color diffuse = Color::White;
    Color specular = Color::White;

    float shininess = 2.0f;
    float dissolve = 1.0f;  // Opacité global [0 = invisible, 1 = opaque]

    // --- Extensions PBR / Shaders Spéciaux ---
    float metallic = 0.0f;   // Facteur métallique [0 = diélectrique, 1 = métal]
    float glass = 0.0f;      // Indice de réflectivité/transparence du verre

    int illum = 1;           // Modèle d'illumination Wavefront MTL

    // --- Texture Albedo ---
    std::shared_ptr<Texture> texture = nullptr;
};