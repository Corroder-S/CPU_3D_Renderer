#pragma once

#include <vector>
#include "Maths/Vec3.h"
#include "Maths/Vec2.h"
#include "Asset/Mesh.h"
#include "Asset/Material.h"

// ============================================================================
// OBJ PARSER INTERFACE
// ============================================================================

/// @brief Charge un fichier mesh Wavefront .obj simple.
bool LoadObj(const char* path, std::vector<Maths::Vec3f>& outVertices, std::vector<Maths::Vec2f>& outuvs, std::vector<Face>& outFaces);

/// @brief Charge un fichier .obj et extrait son matériau principal (.mtl).
bool LoadObj(const char* path, std::vector<Maths::Vec3f>& outVertices, std::vector<Maths::Vec2f>& outuvs, std::vector<Face>& outFaces, Material& outMaterial);