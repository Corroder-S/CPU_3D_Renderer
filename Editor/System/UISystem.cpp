//
// Created by leoam on 04/11/2026.
//

#include "UISystem.h"

#include <functional>

#include "Editor/Component.h"

void UISystem::update(uint32_t* framebuffer, uint32_t fbWidth, uint32_t fbHeight)
{
    m_ui.m_width = fbWidth;
    m_ui.m_height = fbHeight;

    struct DrawCall { uint32_t zOrder; std::function<void()> draw; };
    std::vector<DrawCall> draws;
    m_registry.forEach<CanvaComponent>([&](ECS::EntityID e, CanvaComponent& comp) {
        draws.push_back({comp.canva.zOrder, [&]() {
            m_ui.drawRectangle(comp.canva.x, comp.canva.y, comp.canva.w, comp.canva.h, comp.canva.color, framebuffer);
        }});
    });
    m_registry.forEach<ImageComponent>([&](ECS::EntityID e, ImageComponent& comp) {
        draws.push_back({comp.image.zOrder, [&]() {
            m_ui.drawImage(comp.image.x, comp.image.y, comp.image.w, comp.image.h, comp.image.texture, framebuffer);
        }});
    });
    m_registry.forEach<TextComponent>([&](ECS::EntityID e, TextComponent& comp) {
        draws.push_back({comp.text.zOrder, [&]() {
            m_ui.drawText(*comp.text.font, comp.text.text, comp.text.x, comp.text.y, comp.text.color, framebuffer, fbWidth, fbHeight);
        }});
    });
    std::sort(draws.begin(), draws.end(), [](auto& a, auto& b) { return a.zOrder < b.zOrder; });
    for (auto& d : draws) d.draw();
}