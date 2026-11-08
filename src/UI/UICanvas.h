#pragma once
#include <cstdint>

#include "Asset/Font.h"

struct Texture;

// ============================================================================
// TYPES DE PRIMITIVES UI
// ============================================================================

/// @brief Représente une image (sprite) affichable sur le canvas UI.
struct Image
{
    uint32_t x, y, w, h;       // Position et dimensions en pixels
    const Texture* texture;
    uint32_t zOrder;            // Ordre de dessin (valeur élevée = devant)
};

/// @brief Représente un texte affichable sur le canvas UI.
struct Text
{
    uint32_t x, y;
    Font* font;
    Color color;
    std::string text;
    uint32_t zOrder;            // Ordre de dessin (valeur élevée = devant)
};

/// @brief Représente un rectangle de couleur unie affichable sur le canvas UI.
struct Canva {
    uint32_t x, y, w, h;       // Position et dimensions en pixels
    uint32_t color;             // Couleur ARGB 32 bits
    uint32_t zOrder;            // Ordre de dessin (valeur élevée = devant)
};

// ============================================================================
// UICANVAS
// ============================================================================

/// @brief Gère le dessin des primitives 2D d'interface utilisateur directement dans le framebuffer.
class UICanvas
{
public:

    uint32_t m_width;   // Largeur du framebuffer cible en pixels
    uint32_t m_height;  // Hauteur du framebuffer cible en pixels

    /// @brief Dessine un rectangle plein dans le framebuffer.
    /// @param x, y         Coin supérieur gauche en pixels.
    /// @param width, height Dimensions en pixels.
    /// @param color        Couleur ARGB 32 bits.
    /// @param framebuffer  Buffer destination.
    void drawRectangle(int x, int y, int width, int height, std::uint32_t color, uint32_t* framebuffer);

    /// @brief Dessine une texture dans le framebuffer (copie pixel par pixel).
    /// @param x, y         Coin supérieur gauche en pixels.
    /// @param width, height Dimensions d'affichage en pixels.
    /// @param texture      Texture source à copier.
    /// @param framebuffer  Buffer destination.
    void drawImage(int x, int y, int width, int height, const Texture* texture, uint32_t* framebuffer);

    /// @brief Rend une chaîne de caractères dans le framebuffer avec la police et la couleur données.
    /// @param font        Police bitmap à utiliser.
    /// @param text        Texte à afficher.
    /// @param posX, posY  Position de départ en pixels.
    /// @param color       Couleur du texte.
    /// @param framebuffer Buffer destination.
    /// @param fbWidth     Largeur du framebuffer (pour le clipping).
    /// @param fbHeight    Hauteur du framebuffer (pour le clipping).
    void drawText(const Font& font,const std::string& text,uint32_t posX, uint32_t posY,const Color& color,uint32_t* framebuffer,uint32_t fbWidth, uint32_t fbHeight);


};
