#pragma once
#include <cstdint>
#include <string>

struct EmitterConfig
{
    std::string type;
    float emitRate;
    float maxlife;
    float startSpeed;
    float endSpeed;
    float angleMin;
    float angleMax;
    float spreadAngle;
    float startSize;
    float endSize;
    uint32_t startColor;
    uint32_t endColor;
    uint32_t blendMode;
    float gravityScale = 1.f;
    bool looping = true;

};


EmitterConfig loadFromJson(const std::string& filename);
