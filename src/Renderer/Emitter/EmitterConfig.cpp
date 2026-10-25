#include "EmitterConfig.h"

#include <iostream>
#include <nlohmann/json.hpp>

#include "Platform/MemoryMappedFile.h"

emitterConfig loadFromJson(const std::string& filename)
{
    emitterConfig config;

    MappedFile file(filename.c_str());
    if (!file.IsOpen())
    {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return config;
    }

    nlohmann::json jsonConfig = nlohmann::json::parse(file.Data(), file.Data() + file.Size());

    config.type = jsonConfig.value("type", "default");
    config.emitRate = jsonConfig.value("emitRate", 10);
    config.maxlife = jsonConfig.value("maxlife", 5);
    config.startSpeed = jsonConfig.value("startSpeed", 1);
    config.endSpeed = jsonConfig.value("endSpeed", 0);
    config.angleMin = jsonConfig.value("angleMin", 0);
    config.angleMax = jsonConfig.value("angleMax", 360);
    config.startSize = jsonConfig.value("startSize", 1);
    config.endSize = jsonConfig.value("endSize", 0);
    config.startColor = jsonConfig.value("startColor", 0xFFFFFFFF);
    config.endColor = jsonConfig.value("endColor", 0x00FFFFFF);
    config.blendMode = jsonConfig.value("blendMode", 0);
    config.gravityScale = jsonConfig.value("gravityScale", 1.0f);

    return config;
}
