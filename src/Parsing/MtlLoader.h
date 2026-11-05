#pragma once

#include <string>
#include <unordered_map>
#include "Asset/Material.h"

// ============================================================================
// MTL LOADER INTERFACE
// ============================================================================

namespace MtlLoader {
    /// @brief Charge une bibliothèque de matériaux Wavefront (.mtl).
    /// @param filepath Chemin d'accès au fichier .mtl.
    /// @param baseDir Répertoire racine optionnel pour la résolution des chemins relatifs de textures.
    /// @return Une map associant le nom de chaque matériau à sa structure Material correspondante.
    std::unordered_map<std::string, Material> load(const std::string& filepath, const std::string& baseDir = "");
}