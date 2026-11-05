#pragma once

#include <cstdint>
#include <string>
#include <algorithm>
#include "Asset/Font.h"
#include "Asset/Color.h"

static inline void drawText(
    const Font& font,
    const std::string& text,
    uint32_t posX, uint32_t posY,
    const Color& color,
    uint32_t* framebuffer,
    uint32_t fbWidth, uint32_t fbHeight)
{
    const auto& alphaAtlas = font.getAtlasAlpha();
    uint32_t atlasW = font.getAtlasWidth();
    uint32_t atlasH = font.getAtlasHeight();
    if (alphaAtlas.empty()) return;

    uint8_t cr = static_cast<uint8_t>(std::clamp(color.r * 255.0f, 0.0f, 255.0f));
    uint8_t cg = static_cast<uint8_t>(std::clamp(color.g * 255.0f, 0.0f, 255.0f));
    uint8_t cb = static_cast<uint8_t>(std::clamp(color.b * 255.0f, 0.0f, 255.0f));
    uint8_t ca = static_cast<uint8_t>(std::clamp(color.a * 255.0f, 0.0f, 255.0f));

    float curX = static_cast<float>(posX);
    float curY = static_cast<float>(posY + font.getFontSize());

    for (char c : text) {
        if (c == '\n') {
            curX = static_cast<float>(posX);
            curY += font.getFontSize();
            continue;
        }

        GlyphQuad q;
        font.getBakedQuad(c, curX, curY, q);

        uint32_t w = q.x1 - q.x0;
        uint32_t h = q.y1 - q.y0;

        for (uint32_t dy = 0; dy < h; ++dy) {
            uint32_t screenY = q.y0 + dy;
            if (screenY >= fbHeight) continue;

            float v = q.t0 + (q.t1 - q.t0) * (static_cast<float>(dy) / h);
            uint32_t texY = static_cast<uint32_t>(v * atlasH);

            for (uint32_t dx = 0; dx < w; ++dx) {
                uint32_t screenX = q.x0 + dx;
                if (screenX >= fbWidth) continue;

                float u = q.s0 + (q.s1 - q.s0) * (static_cast<float>(dx) / w);
                uint32_t texX = static_cast<uint32_t>(u * atlasW);

                uint8_t alpha = alphaAtlas[texY * atlasW + texX];
                if (alpha == 0) continue;

                float finalAlpha = (alpha / 255.0f) * (ca / 255.0f);
                uint32_t dstIdx = screenY * fbWidth + screenX;
                uint32_t dstColor = framebuffer[dstIdx];

                uint8_t dr = (dstColor >> 16) & 0xFF;
                uint8_t dg = (dstColor >> 8)  & 0xFF;
                uint8_t db =  dstColor        & 0xFF;

                uint8_t outR = static_cast<uint8_t>(cr * finalAlpha + dr * (1.0f - finalAlpha));
                uint8_t outG = static_cast<uint8_t>(cg * finalAlpha + dg * (1.0f - finalAlpha));
                uint8_t outB = static_cast<uint8_t>(cb * finalAlpha + db * (1.0f - finalAlpha));

                framebuffer[dstIdx] = (0xFFu << 24) | (outR << 16) | (outG << 8) | outB;
            }
        }
    }
}
