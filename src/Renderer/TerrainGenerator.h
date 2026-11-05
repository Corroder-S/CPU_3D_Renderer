#pragma once

#include "Asset/Mesh.h"
#include "Maths/PerlinNoise.h"
#include "Maths/Vec2.h"
#include "Maths/Vec3.h"

#include <vector>
#include <algorithm>
#include <cmath>

enum class TerrainType {
    StandardFBM,
    MountainRidged,
    OrganicWarped,
    DesertDunes,
    TerracedCanyon,
    Island
};

struct TerrainParams {
    int width = 128;
    int depth = 128;
    float cellSize = 0.5f;
    float heightScale = 22.0f;
    float frequency = 0.025f;
    int octaves = 5;
    float persistence = 0.5f;
    float lacunarity = 2.0f;
    float warpStrength = 1.3f;
    unsigned int seed = 2026;
    TerrainType type = TerrainType::MountainRidged;
};

class TerrainGenerator {
public:
    static Mesh Generate(const TerrainParams& params) {
        Mesh mesh;
        PerlinNoise perlin(params.seed);

        int numVerticesX = params.width + 1;
        int numVerticesZ = params.depth + 1;

        float offsetX = (params.width * params.cellSize) * 0.5f;
        float offsetZ = (params.depth * params.cellSize) * 0.5f;

        size_t totalVertices = numVerticesX * numVerticesZ;
        mesh.vertices.reserve(totalVertices);
        mesh.uvs.reserve(totalVertices);
        mesh.colors.reserve(totalVertices);

        auto sampleRidgedFBM = [&](float wx, float wz) -> float {
            float sum = 0.0f;
            float freq = params.frequency;
            float amp = 1.0f;
            float weight = 1.0f;

            for (int i = 0; i < params.octaves; ++i) {
                float n = perlin.noise(wx * freq, wz * freq);
                float signal = 1.0f - std::abs(n);
                signal = signal * signal;

                sum += signal * amp * weight;
                weight = std::clamp(signal * 2.0f, 0.25f, 1.0f);

                freq *= params.lacunarity;
                amp *= params.persistence;
            }

            float warpX = perlin.noise(wx * 0.04f, wz * 0.04f) * 6.0f;
            float warpZ = perlin.noise(wx * 0.04f + 100.0f, wz * 0.04f + 100.0f) * 6.0f;
            float ruggedDetail = perlin.fbm((wx + warpX) * 0.06f, (wz + warpZ) * 0.06f, 3, 0.5f, 2.0f);

            return (sum * 0.70f + ruggedDetail * 0.30f - 0.20f) * params.heightScale;
            };

        auto sampleIslandHeight = [&](float wx, float wz) -> float {
            float wx_c = wx - offsetX;
            float wz_c = wz - offsetZ;
            float dist = std::sqrt(wx_c * wx_c + wz_c * wz_c);
            float maxRadius = std::min(params.width, params.depth) * params.cellSize * 0.45f;

            float mask = std::clamp(1.0f - (dist / maxRadius), 0.0f, 1.0f);
            mask = mask * mask * (3.0f - 2.0f * mask);

            float nx = wx * params.frequency;
            float nz = wz * params.frequency;
            float rawFbm = perlin.domainWarpFbm(nx, nz, params.octaves, params.persistence, params.lacunarity, params.warpStrength);

            float height = (rawFbm * mask) * params.heightScale - 1.0f;
            return (height < -0.3f) ? -0.3f : height;
            };

        auto getTerrainHeight = [&](float wx, float wz) -> float {
            float nx = wx * params.frequency;
            float nz = wz * params.frequency;

            switch (params.type) {
            case TerrainType::MountainRidged:
                return sampleRidgedFBM(wx, wz);
            case TerrainType::OrganicWarped:
                return perlin.domainWarpFbm(nx, nz, params.octaves, params.persistence, params.lacunarity, params.warpStrength) * params.heightScale;
            case TerrainType::Island:
                return sampleIslandHeight(wx, wz);
            case TerrainType::StandardFBM:
            default:
                return perlin.fbm(nx, nz, params.octaves, params.persistence, params.lacunarity) * params.heightScale;
            }
            };

        const float delta = params.cellSize;

        for (int z = 0; z < numVerticesZ; ++z) {
            for (int x = 0; x < numVerticesX; ++x) {
                float worldX = x * params.cellSize;
                float worldZ = z * params.cellSize;
                float worldX_centered = worldX - offsetX;
                float worldZ_centered = worldZ - offsetZ;

                float height = getTerrainHeight(worldX, worldZ);
                Color finalColor{ 1.0f, 1.0f, 1.0f, 1.0f };

                if (params.type == TerrainType::Island) {
                    float hL = getTerrainHeight(worldX - delta, worldZ);
                    float hR = getTerrainHeight(worldX + delta, worldZ);
                    float hD = getTerrainHeight(worldX, worldZ - delta);
                    float hU = getTerrainHeight(worldX, worldZ + delta);
                    Maths::Vec3f normal = Maths::Normalize(Maths::Vec3f{ hL - hR, 2.0f * delta, hD - hU });
                    float slopeAngle = std::acos(std::clamp(normal.y, 0.0f, 1.0f));
                    float slope = slopeAngle / 1.5708f;

                    Color deepWater{ 0.10f, 0.30f, 0.65f, 1.0f };
                    Color shallowWater{ 0.22f, 0.65f, 0.82f, 1.0f };
                    Color sand{ 0.88f, 0.80f, 0.55f, 1.0f };
                    Color grass{ 0.18f, 0.50f, 0.20f, 1.0f };
                    Color rock{ 0.45f, 0.40f, 0.35f, 1.0f };
                    Color snow{ 0.95f, 0.95f, 1.00f, 1.0f };

                    if (height <= -0.2f) {
                        finalColor = deepWater;
                    }
                    else if (height < 0.2f) {
                        float t = std::clamp((height + 0.2f) / 0.4f, 0.0f, 1.0f);
                        finalColor = shallowWater * (1.0f - t) + sand * t;
                    }
                    else if (height < 1.0f) {
                        float t = std::clamp((height - 0.2f) / 0.8f, 0.0f, 1.0f);
                        finalColor = sand * (1.0f - t) + grass * t;
                    }
                    else {
                        float rockWeight = std::clamp((slope - 0.20f) / 0.25f, 0.0f, 1.0f);
                        float normalizedY = std::clamp(height / params.heightScale, 0.0f, 1.0f);
                        float snowWeight = std::clamp((normalizedY - 0.65f) / 0.20f, 0.0f, 1.0f) * (1.0f - rockWeight);
                        float grassWeight = std::clamp(1.0f - rockWeight - snowWeight, 0.0f, 1.0f);

                        finalColor = grass * grassWeight + rock * rockWeight + snow * snowWeight;
                    }
                }
                else if (params.type == TerrainType::DesertDunes) {
                    float nx = worldX * params.frequency;
                    float nz = worldZ * params.frequency;
                    float windAngle = nx * 8.0f + perlin.noise(nx * 2.0f, nz * 2.0f) * 3.0f;
                    float duneWave = std::pow(std::sin(windAngle) * 0.5f + 0.5f, 1.5f);
                    float baseNoise = perlin.fbm(nx * 0.5f, nz * 0.5f, 2, 0.5f, 2.0f);
                    height = duneWave * baseNoise * params.heightScale;

                    Color sandBase{ 0.86f, 0.68f, 0.42f, 1.0f };
                    Color sandCrest{ 0.95f, 0.80f, 0.52f, 1.0f };
                    finalColor = sandBase * (1.0f - duneWave) + sandCrest * duneWave;
                }
                else if (params.type == TerrainType::TerracedCanyon) {
                    float nx = worldX * params.frequency;
                    float nz = worldZ * params.frequency;
                    float rawFbm = perlin.domainWarpFbm(nx, nz, params.octaves, params.persistence, params.lacunarity, params.warpStrength);
                    float steps = 7.0f;
                    float stepped = std::floor(rawFbm * steps);
                    float remainder = (rawFbm * steps) - stepped;
                    float smoothStep = std::pow(remainder, 4.0f);
                    height = ((stepped + smoothStep) / steps) * params.heightScale;

                    float layer = std::fmod(stepped, 3.0f);
                    if (layer < 1.0f)      finalColor = Color{ 0.72f, 0.38f, 0.24f, 1.0f };
                    else if (layer < 2.0f) finalColor = Color{ 0.85f, 0.55f, 0.35f, 1.0f };
                    else                   finalColor = Color{ 0.58f, 0.44f, 0.36f, 1.0f };
                }
                else {
                    float hL = getTerrainHeight(worldX - delta, worldZ);
                    float hR = getTerrainHeight(worldX + delta, worldZ);
                    float hD = getTerrainHeight(worldX, worldZ - delta);
                    float hU = getTerrainHeight(worldX, worldZ + delta);

                    Maths::Vec3f normal = Maths::Normalize(Maths::Vec3f{ hL - hR, 2.0f * delta, hD - hU });

                    float slopeAngle = std::acos(std::clamp(normal.y, 0.0f, 1.0f));
                    float slopeNorm = slopeAngle / 1.5708f;

                    float rockNoise = perlin.noise(worldX * 0.15f, worldZ * 0.15f) * 0.18f;
                    float snowNoise = perlin.noise(worldX * 0.08f + 50.0f, worldZ * 0.08f + 50.0f) * 0.15f;

                    float rockWeight = std::clamp((slopeNorm - (0.12f + rockNoise)) / 0.18f, 0.0f, 1.0f);

                    float normalizedY = std::clamp(height / params.heightScale, 0.0f, 1.0f);
                    float snowThreshold = 0.35f + snowNoise;
                    float snowWeight = std::clamp((normalizedY - snowThreshold) / 0.22f, 0.0f, 1.0f);

                    snowWeight *= (1.0f - rockWeight * 0.8f);

                    float grassWeight = std::clamp(1.0f - rockWeight - snowWeight, 0.0f, 1.0f);

                    Color grassColor{ 0.16f, 0.38f, 0.18f, 1.0f };
                    Color rockColor{ 0.40f, 0.36f, 0.33f, 1.0f };
                    Color snowColor{ 0.95f, 0.96f, 1.00f, 1.0f };

                    finalColor = grassColor * grassWeight + rockColor * rockWeight + snowColor * snowWeight;
                }

                mesh.vertices.push_back({ worldX_centered, height, worldZ_centered });
                mesh.uvs.push_back({ (float)x / params.width, (float)z / params.depth });
                mesh.colors.push_back(finalColor);
            }
        }

        SubMesh submesh;
        submesh.materialName = "TerrainMaterial";

        int numFaces = params.width * params.depth * 2;
        submesh.faces.reserve(numFaces);

        for (int z = 0; z < params.depth; ++z) {
            for (int x = 0; x < params.width; ++x) {
                int topLeft = z * numVerticesX + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * numVerticesX + x;
                int bottomRight = bottomLeft + 1;

                face f1;
                f1.v = Maths::Vec3i{ topLeft, bottomLeft, topRight };
                f1.uv = Maths::Vec3i{ topLeft, bottomLeft, topRight };
                submesh.faces.push_back(f1);

                face f2;
                f2.v = Maths::Vec3i{ topRight, bottomLeft, bottomRight };
                f2.uv = Maths::Vec3i{ topRight, bottomLeft, bottomRight };
                submesh.faces.push_back(f2);
            }
        }

        mesh.submeshes.push_back(submesh);
        mesh.computeNormals(70.0f);

        return mesh;
    }
};