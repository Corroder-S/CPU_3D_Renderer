#pragma once
#include "ECS/headers/ISystem.h"
#include "ECS/headers/registry.h"
#include "Renderer/Renderer.h"


class RendererSystem : public ISystem
{
    ECS::Registry& m_registry;
    Renderer&      m_renderer;

public:
    RendererSystem(ECS::Registry& registry, Renderer& renderer)
        : m_registry(registry), m_renderer(renderer) {}

    void update(float deltaTime) override;

    Renderer &getRenderer() { return m_renderer; }
};
