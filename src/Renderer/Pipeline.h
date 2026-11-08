#pragma once

#include "Maths/Vec2.h"
#include "Maths/Vec3.h"
#include "Maths/Vec4.h"
#include "Maths/Matrix4x4.h"

// ============================================================================
// PIPELINE — TRANSFORMATIONS ET PROJECTION
// ============================================================================

/// @brief Convertit des coordonnées NDC en coordonnées écran (pixels).
/// @param ndcX   Coordonnée X normalisée [-1, 1].
/// @param ndcY   Coordonnée Y normalisée [-1, 1].
/// @param width  Largeur de la fenêtre en pixels.
/// @param height Hauteur de la fenêtre en pixels.
/// @return Coordonnées pixel correspondantes.
static inline Maths::Vec2i viewport(float ndcX, float ndcY, int width, int height)
{
    return Maths::Vec2i(
        (int)((ndcX + 1.0f) * 0.5f * width),
        (int)((1.0f - (ndcY + 1.0f) * 0.5f) * height));
}

/// @brief Projette un sommet 3D en coordonnées écran via la matrice MVP.
/// @param mvp    Matrice Model-View-Projection.
/// @param vertex Sommet 3D en espace objet.
/// @param out    Coordonnées écran résultantes (pixels).
/// @param depth  Profondeur NDC résultante (z/w).
/// @param invW   Inverse du w clip, utilisé pour la correction perspective.
/// @param width  Largeur de la fenêtre en pixels.
/// @param height Hauteur de la fenêtre en pixels.
static inline void projectVertex(const Maths::Matrix4x4<>& mvp, const Maths::Vec3<> vertex,
                                  Maths::Vec2i& out, float& depth, float& invW,
                                  int width, int height)
{
    Maths::Vec4<> clip = mvp * Maths::Vec4<>(vertex, 1.0f);
    Maths::Vec3<> ndc  = { clip.x / clip.w, clip.y / clip.w, clip.z / clip.w };
    out   = viewport(ndc.x, ndc.y, width, height);
    depth = ndc.z;
    invW  = 1.0f / clip.w;
}

/// @brief Projette un tableau de sommets 3D en coordonnées écran via la matrice MVP.
/// @param mvp         Matrice Model-View-Projection.
/// @param vertices    Tableau de sommets 3D en espace objet.
/// @param vertexCount Nombre de sommets à projeter.
/// @param out         Tableau de coordonnées écran résultantes.
/// @param depths      Tableau des profondeurs NDC résultantes.
/// @param invWs       Tableau des inverses du w clip.
/// @param width       Largeur de la fenêtre en pixels.
/// @param height      Hauteur de la fenêtre en pixels.
static inline void projectMesh(const Maths::Matrix4x4<>& mvp, const Maths::Vec3<>* vertices,
                                 int vertexCount, Maths::Vec2i* out, float* depths, float* invWs,
                                 int width, int height)
{
    for (int i = 0; i < vertexCount; i++)
        projectVertex(mvp, vertices[i], out[i], depths[i], invWs[i], width, height);
}

/// @brief Calcule la luminosité d'une surface à partir du modèle diffus + ambiant.
/// @param normal         Normale de surface (normalisée).
/// @param lightDirection Direction vers la lumière (normalisée).
/// @param ambient        Facteur d'éclairage ambiant [0, 1].
/// @param outBrightness  Luminosité résultante clampée dans [0, 1].
static inline void computeBrightness(Maths::Vec3f& normal, Maths::Vec3f lightDirection,  float ambient, float& outBrightness)
{
    float diffuse = std::max(0.0f,Maths::Dot(normal, lightDirection));

    float brightness = ambient + (1 - ambient) * diffuse;
    outBrightness = std::clamp(brightness, 0.0f,1.0f);
}
