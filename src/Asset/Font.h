#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "external/stb/stb_truetype.h"
#include "Asset/Color.h"

// ============================================================================
// FONT STRUCTURES & CLASS
// ============================================================================

/// @brief Quad de rendu pour un glyphe (coordonnées écran et UVs atlas).
struct GlyphQuad {
    int x0, y0, x1, y1;
    float s0, t0, s1, t1;
};

/// @brief Charge et gère les polices TTF via un atlas de texture Alpha.
class Font {
public:
    Font() = default;
    ~Font() = default;

    /// @brief Charge un fichier .ttf et génère l'atlas rasterisé.
    bool load(const std::string& filepath, float fontSize = 18.0f);

    // --- Accessors ---
    const std::vector<uint8_t>& getAtlasAlpha() const { return m_atlasAlpha; }
    int getAtlasWidth() const { return m_atlasWidth; }
    int getAtlasHeight() const { return m_atlasHeight; }
    float getFontSize() const { return m_fontSize; }

    /// @brief Calcule le quad de rendu écran et fait avancer le curseur de texte.
    void getBakedQuad(char c, float& x, float& y, GlyphQuad& outQuad) const;

private:
    int m_atlasWidth = 512;
    int m_atlasHeight = 512;
    float m_fontSize = 18.0f;

    std::vector<uint8_t> m_atlasAlpha;
    stbtt_bakedchar m_cdata[96]; // Caractères ASCII imprimables (32..127)
};