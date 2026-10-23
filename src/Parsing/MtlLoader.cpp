#include "MtlLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include "Asset/Texture.h"

namespace MtlLoader {

    std::unordered_map<std::string, Material> load(const std::string& filepath, const std::string& baseDir)
    {
        std::unordered_map<std::string, Material> materials;
        std::ifstream file(filepath);

        if (!file.is_open()) {
            std::cerr << "MtlLoader : Impossible d'ouvrir " << filepath << std::endl;
            return materials;
        }

        std::string line;
        std::string currentMtl;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string keyword;
            ss >> keyword;

            if (keyword == "newmtl") {
                ss >> currentMtl;
                materials[currentMtl] = Material{};
            }
            else if (currentMtl.empty()) {
                continue;
            }
            else if (keyword == "Ka") {
                float r, g, b;
                ss >> r >> g >> b;
                materials[currentMtl].ambient = Color(r, g, b, 1.0f);
            }
            else if (keyword == "Kd") {
                float r, g, b;
                ss >> r >> g >> b;
                materials[currentMtl].diffuse = Color(r, g, b, 1.0f);
            }
            else if (keyword == "Ks") {
                float r, g, b;
                ss >> r >> g >> b;
                materials[currentMtl].specular = Color(r, g, b, 1.0f);
            }
            else if (keyword == "Ns") {
                float shininess;
                ss >> shininess;
                materials[currentMtl].shininess = shininess;
            }
            else if (keyword == "d") {
                float dissolve;
                ss >> dissolve;
                materials[currentMtl].dissolve = dissolve;
            }
            else if (keyword == "Tr") {
                float tr;
                ss >> tr;
                materials[currentMtl].dissolve = 1.0f - tr;
            }
            else if (keyword == "map_Kd") {
                std::string texName;
                ss >> texName;
                std::string fullPath = baseDir.empty() ? texName : baseDir + "/" + texName;

                auto tex = std::make_shared<Texture>();
                if (tex->load(fullPath)) {
                    materials[currentMtl].texture = tex;
                }
            }
        }

        return materials;
    }

}