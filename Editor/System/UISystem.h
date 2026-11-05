#pragma once
#include "ECS/headers/registry.h"
#include "UI/UICanvas.h"

class UISystem
{
    ECS::Registry&  m_registry;
    UICanvas m_ui;
public:
    UISystem(ECS::Registry& registry) : m_registry(registry) {}
    void update(uint32_t* framebuffer, uint32_t fbWidth, uint32_t fbHeight);
};
