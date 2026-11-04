#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <cstdint>
#include "Maths/Vec2.h"
#include "Maths/Vec3.h"
#include "Material.h"

struct face {
    Maths::Vec3i v;
    Maths::Vec3i uv;
};

struct Mesh {
    std::vector<Maths::Vec3f> vertices;
    std::vector<Maths::Vec2f> uvs;
    std::vector<face>         faces;
    std::vector<Maths::Vec3f> faceNormals;
    std::vector<Maths::Vec3f> cornerNormals;

    void computeNormals(float angleDeg = 50.0f)
    {
        const size_t F = faces.size();
        const float cosT = std::cos(angleDeg * 3.14159265f / 180.0f);

        std::vector<Maths::Vec3f> fn(F);
        std::vector<std::vector<uint32_t>> adj(vertices.size());

        for (size_t i = 0; i < F; ++i) {
            const auto& f = faces[i];
            const auto& p0 = vertices[f.v.x];
            const auto& p1 = vertices[f.v.z];
            const auto& p2 = vertices[f.v.y];

            fn[i] = Maths::Normalize(Maths::Cross(p1 - p0, p2 - p0));
            adj[f.v.x].push_back((uint32_t)i);
            adj[f.v.y].push_back((uint32_t)i);
            adj[f.v.z].push_back((uint32_t)i);
        }

        cornerNormals.resize(F * 3);
        for (size_t i = 0; i < F; ++i) {
            const auto& f = faces[i];
            const int corner[3] = { f.v.x, f.v.z, f.v.y };
            for (int c = 0; c < 3; ++c) {
                Maths::Vec3f sum = { 0, 0, 0 };
                for (uint32_t j : adj[corner[c]])
                    if (Maths::Dot(fn[i], fn[j]) >= cosT)
                        sum = sum + fn[j];
                Maths::Vec3f n = Maths::Normalize(sum);
                cornerNormals[i * 3 + c] = (Maths::Dot(sum, sum) > 1e-12f) ? n : fn[i];
            }
        }
    }
};


Mesh LoadMesh(const char* path, Material& material);
Mesh LoadMesh(const char* path);