#pragma once
#include <cstdint>
#include <string>

// ============================================================================
// CONFIGURATION D'ÉMETTEUR
// ============================================================================

/// @brief Paramètres de configuration d'un émetteur de particules.
/// Peut être chargée depuis un fichier JSON via loadFromJson.
struct EmitterConfig
{
    std::string type;         ///< Type d'émetteur (ex. "point", "sphere").
    float emitRate;           ///< Nombre de particules émises par seconde.
    float maxlife;            ///< Durée de vie maximale d'une particule (secondes).
    float startSpeed;         ///< Vitesse initiale des particules.
    float endSpeed;           ///< Vitesse en fin de vie des particules.
    float angleMin;           ///< Angle minimal d'émission (radians).
    float angleMax;           ///< Angle maximal d'émission (radians).
    float spreadAngle;        ///< Angle d'ouverture du cône d'émission (radians).
    float startSize;          ///< Taille initiale des particules.
    float endSize;            ///< Taille en fin de vie des particules.
    uint32_t startColor;      ///< Couleur initiale (format 0xAARRGGBB).
    uint32_t endColor;        ///< Couleur en fin de vie (format 0xAARRGGBB).
    uint32_t blendMode;       ///< Mode de fusion (0 = alpha, 1 = additif, etc.).
    float gravityScale = 1.f; ///< Multiplicateur de gravité appliqué aux particules.
    bool looping = true;      ///< Si vrai, l'émetteur émet en boucle indéfiniment.
};

/// @brief Charge une configuration d'émetteur depuis un fichier JSON.
/// @param filename Chemin vers le fichier JSON.
/// @return Configuration remplie.
EmitterConfig loadFromJson(const std::string& filename);
