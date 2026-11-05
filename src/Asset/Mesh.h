#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <cstdint>
#include "Maths/Vec2.h"
#include "Maths/Vec3.h"
#include "Material.h"

struct Face {
    Maths::Vec3i v;
    Maths::Vec3i uv;
};

struct SubMesh {
    std::string materialName;
    Material material;
    std::vector<Face> faces;
    std::vector<Maths::Vec3f> cornerNormals;
};

struct Mesh {
    std::vector<Maths::Vec3f> vertices;
    std::vector<Maths::Vec2f> uvs;
    std::vector<SubMesh>      submeshes;

    void computeNormals(float angleDeg = 50.0f)
    {
        size_t totalFaces = 0;
        for (const auto& sm : submeshes) {
            totalFaces += sm.faces.size();
        }
        if (totalFaces == 0 || vertices.empty()) return;

        const float cosT = std::cos(angleDeg * 3.14159265f / 180.0f);
        std::vector<std::vector<uint32_t>> adj(vertices.size());

        struct FlatFaceRef {
            size_t submeshIdx;
            size_t faceIdx;
            Maths::Vec3f normal;
        };

        std::vector<FlatFaceRef> flatFaces;
        flatFaces.reserve(totalFaces);

        // 1. Calcul des normales de face et construction de la table d'adjacence
        for (size_t smIdx = 0; smIdx < submeshes.size(); ++smIdx) {
            for (size_t fIdx = 0; fIdx < submeshes[smIdx].faces.size(); ++fIdx) {
                const auto& f = submeshes[smIdx].faces[fIdx];
                const auto& p0 = vertices[f.v.x];
                const auto& p1 = vertices[f.v.z];
                const auto& p2 = vertices[f.v.y];

                Maths::Vec3f fn = Maths::Normalize(Maths::Cross(p1 - p0, p2 - p0));
                uint32_t flatIdx = static_cast<uint32_t>(flatFaces.size());
                flatFaces.push_back({ smIdx, fIdx, fn });

                adj[f.v.x].push_back(flatIdx);
                adj[f.v.y].push_back(flatIdx);
                adj[f.v.z].push_back(flatIdx);
            }
        }

        // 2. Allocation et calcul des normales aux sommets par SubMesh
        for (auto& sm : submeshes) {
            sm.cornerNormals.resize(sm.faces.size() * 3);
        }

        for (size_t flatIdx = 0; flatIdx < flatFaces.size(); ++flatIdx) {
            const auto& ref = flatFaces[flatIdx];
            const auto& f = submeshes[ref.submeshIdx].faces[ref.faceIdx];
            const int corner[3] = { f.v.x, f.v.z, f.v.y };

            for (int c = 0; c < 3; ++c) {
                Maths::Vec3f sum = { 0, 0, 0 };
                for (uint32_t j : adj[corner[c]]) {
                    if (Maths::Dot(ref.normal, flatFaces[j].normal) >= cosT) {
                        sum = sum + flatFaces[j].normal;
                    }
                }
                Maths::Vec3f n = Maths::Normalize(sum);
                submeshes[ref.submeshIdx].cornerNormals[ref.faceIdx * 3 + c] =
                    (Maths::Dot(sum, sum) > 1e-12f) ? n : ref.normal;
            }
        }
    }
};

Mesh LoadMesh(const char* path);
Mesh LoadMesh(const char* path, Material& material);