#pragma once

#include "Maths/Vec2.h"
#include "Maths/Vec3.h"
#include "Maths/Vec4.h"
#include "Asset/Color.h"

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/// @brief Sommet en espace de clippage avec attributs interpolables.
struct ClipVertex {
    Maths::Vec4f clip;      // Position homogène (x, y, z, w)
    Maths::Vec2f uv;        // Coordonnées de texture
    Maths::Vec3f normal;    // Normale en espace monde
    Maths::Vec3f worldPos;  // Position absolue en espace monde
    Color color;            // Couleur attribuée au sommet

    /// @brief Interpole tous les attributs entre deux sommets.
    static ClipVertex lerp(const ClipVertex& a, const ClipVertex& b, float t) {
        return {
            a.clip + (b.clip - a.clip) * t,
            a.uv + (b.uv - a.uv) * t,
            a.normal + (b.normal - a.normal) * t,
            a.worldPos + (b.worldPos - a.worldPos) * t,
            Color::lerp(a.color, b.color, t)
        };
    }

    /// @brief Calcule la distance du sommet par rapport au plan Near (z + w >= 0).
    float nearDistance() const {
        return clip.z + clip.w;
    }
};

// ============================================================================
// CLIPPING ALGORITHMS
// ============================================================================

/// @brief Découpe un triangle contre le plan Near (Sutherland-Hodgman).
/// @param v0, v1, v2 Les trois sommets du triangle original.
/// @param out Tableau de sortie recevant les triangles découpés (max 2 triangles, soit 6 sommets).
/// @return Le nombre de triangles générés (0, 1 ou 2).
inline int clipTriangleNear(const ClipVertex& v0, const ClipVertex& v1, const ClipVertex& v2, ClipVertex out[6]) {
    const ClipVertex in[3] = { v0, v1, v2 };
    ClipVertex poly[4];
    int n = 0;

    // 1. Traitement des arêtes du triangle contre le plan Near
    for (int e = 0; e < 3; ++e) {
        const ClipVertex& a = in[e];
        const ClipVertex& b = in[(e + 1) % 3];

        float da = a.nearDistance();
        float db = b.nearDistance();
        bool aIn = (da >= 0.0f);
        bool bIn = (db >= 0.0f);

        if (aIn && bIn) {
            poly[n++] = b;
        }
        else if (aIn && !bIn) {
            float t = da / (da - db);
            poly[n++] = ClipVertex::lerp(a, b, t);
        }
        else if (!aIn && bIn) {
            float t = da / (da - db);
            poly[n++] = ClipVertex::lerp(a, b, t);
            poly[n++] = b;
        }
    }

    // 2. Assemblage des nouveaux triangles issus du polygone découpé
    if (n == 3) {
        out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
        return 1;
    }
    if (n == 4) {
        out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
        out[3] = poly[0]; out[4] = poly[2]; out[5] = poly[3];
        return 2;
    }

    return 0;
}