#pragma once
#include <cstdint>
#include <vector>

class UIElement {
public:
    virtual ~UIElement() = default;
    virtual void draw(std::vector<uint32_t>& framebuffer, uint32_t fbWidth, uint32_t fbHeight) = 0;

    bool visible = true;
    int x = 0;
    int y = 0;
};