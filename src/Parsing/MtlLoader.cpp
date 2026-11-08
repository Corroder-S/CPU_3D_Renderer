#include "MtlLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <vector>
#include "Asset/Texture.h"

namespace MtlLoader {

    // ============================================================================
    // MATERIAL FILE PARSER
    // ============================================================================

    std::unordered_map<std::string, Material> load(const std::string& filepath, const std::string& baseDir)
    {
        std::unordered_map<std::string, Material> materials;
        std::ifstream file(filepath);

        if (!file.is_open()) {
            std::cerr << "MtlLoader : Impossible d'ouvrir " << filepath << std::endl;
            return materials;
        }

        // 1. Résolution des dossiers racines pour les textures relatives
        std::string mtlDir = baseDir;
        if (mtlDir.empty()) {
            size_t lastSlash = filepath.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                mtlDir = filepath.substr(0, lastSlash);
            }
        }

        std::string assetRootDir = mtlDir;
        size_t parentSlash = mtlDir.find_last_of("/\\");
        if (parentSlash != std::string::npos) {
            assetRootDir = mtlDir.substr(0, parentSlash);
        }

        std::string line;
        std::string currentMtl;
        std::unordered_set<std::string> pbrMaterials;

        // 2. Parsing du fichier ligne par ligne
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string keyword;
            ss >> keyword;

            // Déclaration d'un nouveau matériau
            if (keyword == "newmtl") {
                ss >> currentMtl;
                materials[currentMtl] = Material{};
            }
            else if (currentMtl.empty()) {
                continue;
            }
            // Composante Ambiante
            else if (keyword == "Ka") {
                float r, g, b;
                ss >> r >> g >> b;
                materials[currentMtl].ambient = Color(r, g, b, 1.0f);
            }
            // Composante Diffuse
            else if (keyword == "Kd") {
                float r, g, b;
                ss >> r >> g >> b;
                materials[currentMtl].diffuse = Color(r, g, b, 1.0f);
            }
            // Composante Spéculaire
            else if (keyword == "Ks") {
                float r, g, b;
                ss >> r >> g >> b;
                materials[currentMtl].specular = Color(r, g, b, 1.0f);
            }
            // Exposant Spéculaire (Shininess)
            else if (keyword == "Ns") {
                float shininess;
                ss >> shininess;
                materials[currentMtl].shininess = shininess;
            }
            // Opacité (Dissolve)
            else if (keyword == "d") {
                float dissolve;
                ss >> dissolve;
                materials[currentMtl].dissolve = dissolve;
            }
            // Transparence
            else if (keyword == "Tr") {
                float tr;
                ss >> tr;
                materials[currentMtl].dissolve = 1.0f - tr;
            }
            // Propriété PBR Métallique
            else if (keyword == "Pm") {
                float m;
                ss >> m;
                materials[currentMtl].metallic = std::clamp(m, 0.0f, 1.0f);
                pbrMaterials.insert(currentMtl);
            }
            // Propriété PBR Rugosité (Roughness -> conversion vers Shininess)
            else if (keyword == "Pr") {
                float r;
                ss >> r;
                r = std::clamp(r, 0.03f, 1.0f);
                materials[currentMtl].shininess = std::min(2.0f / (r * r) - 2.0f, 2048.0f);
                pbrMaterials.insert(currentMtl);
            }
            // Filtre de Transmission (Rendu Verre)
            else if (keyword == "Tf") {
                float r = 1.0f, g = 1.0f, b = 1.0f;
                if (ss >> r >> g >> b) {
                    float avgTransmission = (r + g + b) / 3.0f;
                    if (avgTransmission > 0.0f && materials[currentMtl].dissolve >= 1.0f) {
                        materials[currentMtl].dissolve = 1.0f - (avgTransmission * 0.6f);
                        materials[currentMtl].glass = 1.0f;
                    }
                }
            }
            // Map Diffuse (Texture)
            else if (keyword == "map_Kd") {
                std::string rawTexPath;
                std::getline(ss >> std::ws, rawTexPath);
                if (!rawTexPath.empty() && rawTexPath.back() == '\r') {
                    rawTexPath.pop_back();
                }

                std::replace(rawTexPath.begin(), rawTexPath.end(), '\\', '/');

                std::string fileNameOnly = rawTexPath;
                size_t slashPos = rawTexPath.find_last_of('/');
                if (slashPos != std::string::npos) {
                    fileNameOnly = rawTexPath.substr(slashPos + 1);
                }

                // Chemins candidats pour localiser la texture
                std::vector<std::string> candidatePaths;
                candidatePaths.push_back(assetRootDir + "/Textures/" + fileNameOnly);
                candidatePaths.push_back(mtlDir + "/" + rawTexPath);
                candidatePaths.push_back(mtlDir + "/" + fileNameOnly);
                candidatePaths.push_back(assetRootDir + "/Image/" + fileNameOnly);
                candidatePaths.push_back(rawTexPath);
                candidatePaths.push_back(fileNameOnly);

                auto tex = std::make_shared<Texture>();
                bool loaded = false;

                // 3. Recherche et chargement de la texture sur disque
                for (const auto& path : candidatePaths) {
                    if (tex->load(path)) {
                        materials[currentMtl].texture = tex;
                        loaded = true;
                        break;
                    }
                }

                if (!loaded) {
                    std::cerr << "MtlLoader : Texture introuvable '" << rawTexPath
                        << "' (matériau '" << currentMtl << "')" << std::endl;
                }
            }
        }

        // 4. Linear-to-sRGB pré-conversion pour les matériaux identifiés comme PBR
        for (const auto& name : pbrMaterials) {
            Color& d = materials[name].diffuse;
            d.r = std::sqrt(d.r);
            d.g = std::sqrt(d.g);
            d.b = std::sqrt(d.b);
        }

        return materials;
    }

}