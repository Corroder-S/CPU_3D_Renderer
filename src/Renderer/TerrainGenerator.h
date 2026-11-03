#pragma once

#include "Asset/Mesh.h"
#include "Maths/PerlinNoise.h"
#include "Maths/Vec2.h"
#include "Maths/Vec3.h"

#include <vector>

struct TerrainParams {
    int width = 64;
    int depth = 64;
    float cellSize = 1.0f;
    float heightScale = 10.0f;
    float frequency = 0.03f;
    int octaves = 4;
    float persistence = 0.5f;
    float lacunarity = 2.0f;
    unsigned int seed = 2026;
};

class TerrainGenerator {
public:
    static Mesh Generate(const TerrainParams& params) {
        Mesh mesh;
        PerlinNoise perlin(params.seed);

        int numVerticesX = params.width + 1;
        int numVerticesZ = params.depth + 1;

        auto GetHeight = [&](float x, float z) -> float {
            float n = perlin.fbm(x * params.frequency, z * params.frequency,
                params.octaves, params.persistence, params.lacunarity);
            return n * params.heightScale;
            };

        float offsetX = (params.width * params.cellSize) * 0.5f;
        float offsetZ = (params.depth * params.cellSize) * 0.5f;

        mesh.vertices.reserve(numVerticesX * numVerticesZ);
        mesh.uvs.reserve(numVerticesX * numVerticesZ);

        for (int z = 0; z < numVerticesZ; ++z) {
            for (int x = 0; x < numVerticesX; ++x) {
                float worldX = x * params.cellSize;
                float worldZ = z * params.cellSize;
                float height = GetHeight(worldX, worldZ);

                mesh.vertices.push_back({ worldX - offsetX, height, worldZ - offsetZ });
                mesh.uvs.push_back({ (float)x / params.width, (float)z / params.depth });
            }
        }

        int numFaces = params.width * params.depth * 2;
        mesh.faces.reserve(numFaces);

        for (int z = 0; z < params.depth; ++z) {
            for (int x = 0; x < params.width; ++x) {
                int topLeft = z * numVerticesX + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * numVerticesX + x;
                int bottomRight = bottomLeft + 1;


                face f1;
                f1.v = Maths::Vec3i{ topLeft, bottomLeft, topRight };
                f1.uv = Maths::Vec3i{ topLeft, bottomLeft, topRight };
                mesh.faces.push_back(f1);


                face f2;
                f2.v = Maths::Vec3i{ topRight, bottomLeft, bottomRight };
                f2.uv = Maths::Vec3i{ topRight, bottomLeft, bottomRight };
                mesh.faces.push_back(f2);
            }
        }

        mesh.faceNormals.resize(mesh.faces.size());
        mesh.vertexNormals.assign(mesh.vertices.size(), Maths::Vec3f{ 0.0f, 0.0f, 0.0f });

        for (size_t i = 0; i < mesh.faces.size(); ++i) {
            const Maths::Vec3f& v0 = mesh.vertices[mesh.faces[i].v.x];
            const Maths::Vec3f& v1 = mesh.vertices[mesh.faces[i].v.y];
            const Maths::Vec3f& v2 = mesh.vertices[mesh.faces[i].v.z];

            mesh.faceNormals[i] = Maths::Normalize(Maths::Cross(v2 - v0, v1 - v0));

            mesh.vertexNormals[mesh.faces[i].v.x] += mesh.faceNormals[i];
            mesh.vertexNormals[mesh.faces[i].v.y] += mesh.faceNormals[i];
            mesh.vertexNormals[mesh.faces[i].v.z] += mesh.faceNormals[i];
        }

        for (auto& n : mesh.vertexNormals) {
            n = Maths::Normalize(n);
        }

        return mesh;
    }
};