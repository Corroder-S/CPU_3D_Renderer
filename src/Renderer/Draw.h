#pragma once


#include <cstdint>

#include "Pipeline.h"
#include "Shader.h"
#include "Maths/Matrix4x4.h"
#include "Framebuffer.h"
#include "Maths/Vec2.h"
#include "Asset/Material.h"
#include "Asset/Light.h"
#include "Asset/Font.h"



static inline void drawline(int x0, int y0, int x1, int y1, uint32_t color, std::vector<uint32_t>& framebuffer)
{
    int dx =  abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;

    while (true)
    {
        put_pixel(x0, y0, color, framebuffer, RENDER_WIDTH, RENDER_HEIGHT);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static inline int edge(Maths::Vec3f a, Maths::Vec3f b, Maths::Vec3f c)
{
    return (int) (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static inline int edge_dxdy(int a, int b)
{
    return (int) (a - b);
}


static inline void drawTriangle(
    Maths::Vec3f a, Maths::Vec3f b, Maths::Vec3f c,
    Maths::Vec2f uvx, Maths::Vec2f uvy, Maths::Vec2f uvz,
    Maths::Vec3f invW,
    Maths::Vec3f wn0, Maths::Vec3f wn1, Maths::Vec3f wn2,
    Maths::Vec3f wp0, Maths::Vec3f wp1, Maths::Vec3f wp2,
    std::vector<uint32_t>& framebuffer,
    std::vector<float>& zbuffer,
    const ShaderUniforms& uniforms,
    const Light& light, Maths::Vec2i renderMin, Maths::Vec2i renderMax,
    bool writeDepth = true)
{


    int minX = std::max(renderMin.x, std::min({ (int)a.x, (int)b.x, (int)c.x }));
    int maxX = std::min(renderMax.x - 1, std::max({ (int)a.x, (int)b.x, (int)c.x }));
    int minY = std::max(renderMin.y, std::min({ (int)a.y, (int)b.y, (int)c.y }));
    int maxY = std::min(renderMax.y - 1, std::max({ (int)a.y, (int)b.y, (int)c.y }));

    Maths::Vec3f minPos = { (float)minX, (float)minY, 0.0f };

    int w0_row = edge(b, c, minPos);
    int w1_row = edge(c, a, minPos);
    int w2_row = edge(a, b, minPos);

    int area = w0_row + w1_row + w2_row;
    float invArea = 1.0f / (float)area;

    int dw0_dx = edge_dxdy((int)b.y, (int)c.y);
    int dw0_dy = edge_dxdy((int)c.x, (int)b.x);
    int dw1_dx = edge_dxdy((int)c.y, (int)a.y);
    int dw1_dy = edge_dxdy((int)a.x, (int)c.x);
    int dw2_dx = edge_dxdy((int)a.y, (int)b.y);
    int dw2_dy = edge_dxdy((int)b.x, (int)a.x);

    float dfw0_dx = dw0_dx * invArea, dfw0_dy = dw0_dy * invArea;
    float dfw1_dx = dw1_dx * invArea, dfw1_dy = dw1_dy * invArea;
    float dfw2_dx = dw2_dx * invArea, dfw2_dy = dw2_dy * invArea;

    float dz_dx = dfw0_dx * a.z + dfw1_dx * b.z + dfw2_dx * c.z;
    float dz_dy = dfw0_dy * a.z + dfw1_dy * b.z + dfw2_dy * c.z;

    float fw0_row = w0_row * invArea;
    float fw1_row = w1_row * invArea;
    float fw2_row = w2_row * invArea;

    // Interpolation UVs et 1/W
    float uw_row = fw0_row * uvx.x * invW.x + fw1_row * uvy.x * invW.y + fw2_row * uvz.x * invW.z;
    float vw_row = fw0_row * uvx.y * invW.x + fw1_row * uvy.y * invW.y + fw2_row * uvz.y * invW.z;
    float iw_row = fw0_row * invW.x + fw1_row * invW.y + fw2_row * invW.z;

    float duw_dx = dfw0_dx * uvx.x * invW.x + dfw1_dx * uvy.x * invW.y + dfw2_dx * uvz.x * invW.z;
    float dvw_dx = dfw0_dx * uvx.y * invW.x + dfw1_dx * uvy.y * invW.y + dfw2_dx * uvz.y * invW.z;
    float diw_dx = dfw0_dx * invW.x + dfw1_dx * invW.y + dfw2_dx * invW.z;

    float duw_dy = dfw0_dy * uvx.x * invW.x + dfw1_dy * uvy.x * invW.y + dfw2_dy * uvz.x * invW.z;
    float dvw_dy = dfw0_dy * uvx.y * invW.x + dfw1_dy * uvy.y * invW.y + dfw2_dy * uvz.y * invW.z;
    float diw_dy = dfw0_dy * invW.x + dfw1_dy * invW.y + dfw2_dy * invW.z;

    float z_row = fw0_row * a.z + fw1_row * b.z + fw2_row * c.z;

    // Interpolation des Normales
    float dnx_dx = dfw0_dx * wn0.x + dfw1_dx * wn1.x + dfw2_dx * wn2.x;
    float dny_dx = dfw0_dx * wn0.y + dfw1_dx * wn1.y + dfw2_dx * wn2.y;
    float dnz_dx = dfw0_dx * wn0.z + dfw1_dx * wn1.z + dfw2_dx * wn2.z;
    float dnx_dy = dfw0_dy * wn0.x + dfw1_dy * wn1.x + dfw2_dy * wn2.x;
    float dny_dy = dfw0_dy * wn0.y + dfw1_dy * wn1.y + dfw2_dy * wn2.y;
    float dnz_dy = dfw0_dy * wn0.z + dfw1_dy * wn1.z + dfw2_dy * wn2.z;

    float nx_row = fw0_row * wn0.x + fw1_row * wn1.x + fw2_row * wn2.x;
    float ny_row = fw0_row * wn0.y + fw1_row * wn1.y + fw2_row * wn2.y;
    float nz_row = fw0_row * wn0.z + fw1_row * wn1.z + fw2_row * wn2.z;

    // Interpolation des Positions Monde
    float pxw_row = fw0_row * wp0.x * invW.x + fw1_row * wp1.x * invW.y + fw2_row * wp2.x * invW.z;
    float pyw_row = fw0_row * wp0.y * invW.x + fw1_row * wp1.y * invW.y + fw2_row * wp2.y * invW.z;
    float pzw_row = fw0_row * wp0.z * invW.x + fw1_row * wp1.z * invW.y + fw2_row * wp2.z * invW.z;

    float dpxw_dx = dfw0_dx * wp0.x * invW.x + dfw1_dx * wp1.x * invW.y + dfw2_dx * wp2.x * invW.z;
    float dpyw_dx = dfw0_dx * wp0.y * invW.x + dfw1_dx * wp1.y * invW.y + dfw2_dx * wp2.y * invW.z;
    float dpzw_dx = dfw0_dx * wp0.z * invW.x + dfw1_dx * wp1.z * invW.y + dfw2_dx * wp2.z * invW.z;

    float dpxw_dy = dfw0_dy * wp0.x * invW.x + dfw1_dy * wp1.x * invW.y + dfw2_dy * wp2.x * invW.z;
    float dpyw_dy = dfw0_dy * wp0.y * invW.x + dfw1_dy * wp1.y * invW.y + dfw2_dy * wp2.y * invW.z;
    float dpzw_dy = dfw0_dy * wp0.z * invW.x + dfw1_dy * wp1.z * invW.y + dfw2_dy * wp2.z * invW.z;

    int idx = minY * RENDER_WIDTH + minX;
    for (int y = minY; y <= maxY; y++)
    {
        int w0 = w0_row, w1 = w1_row, w2 = w2_row;
        float uw = uw_row, vw = vw_row, iw = iw_row;
        float z = z_row;
        float nx = nx_row, ny = ny_row, nz = nz_row;
        float pxw = pxw_row, pyw = pyw_row, pzw = pzw_row;

        for (int x = minX; x <= maxX; x++)
        {
            if ((w0 | w1 | w2) >= 0 && z <= zbuffer[idx])
            {
                float inv_iw = 1.0f / iw;

                FragmentInput frag;
                frag.uv = { uw * inv_iw, vw * inv_iw };
                frag.normal = { nx, ny, nz };
                frag.worldPos = { pxw * inv_iw, pyw * inv_iw, pzw * inv_iw };

                Color color = evaluateBlinnPhong(frag, uniforms);

                if (color.a < 1.0f)
                {
                    uint32_t dst = framebuffer[idx];
                    uint8_t dr = (dst >> 16) & 0xFF;
                    uint8_t dg = (dst >> 8) & 0xFF;
                    uint8_t db = dst & 0xFF;

                    uint8_t sr = static_cast<uint8_t>(std::clamp(color.r * 255.0f, 0.0f, 255.0f));
                    uint8_t sg = static_cast<uint8_t>(std::clamp(color.g * 255.0f, 0.0f, 255.0f));
                    uint8_t sb = static_cast<uint8_t>(std::clamp(color.b * 255.0f, 0.0f, 255.0f));
                    float a = color.a;

                    uint8_t outR = static_cast<uint8_t>(sr * a + dr * (1.0f - a));
                    uint8_t outG = static_cast<uint8_t>(sg * a + dg * (1.0f - a));
                    uint8_t outB = static_cast<uint8_t>(sb * a + db * (1.0f - a));

                    framebuffer[idx] = (0xFFu << 24) | (outR << 16) | (outG << 8) | outB;
                }
                else
                {
                    framebuffer[idx] = color.toRGBA32();
                }

                if (writeDepth)
                {
                    zbuffer[idx] = z;
                }
            }

            w0 += dw0_dx; w1 += dw1_dx; w2 += dw2_dx;
            z += dz_dx;
            uw += duw_dx; vw += dvw_dx; iw += diw_dx;
            nx += dnx_dx; ny += dny_dx; nz += dnz_dx;
            pxw += dpxw_dx; pyw += dpyw_dx; pzw += dpzw_dx;

            idx++;
        }

        idx += RENDER_WIDTH - (maxX - minX + 1);
        z_row += dz_dy;
        uw_row += duw_dy; vw_row += dvw_dy; iw_row += diw_dy;
        nx_row += dnx_dy; ny_row += dny_dy; nz_row += dnz_dy;
        pxw_row += dpxw_dy; pyw_row += dpyw_dy; pzw_row += dpzw_dy;
        w0_row += dw0_dy; w1_row += dw1_dy; w2_row += dw2_dy;
    }
}

static inline void drawText(
    const Font& font,
    const std::string& text,
    int posX, int posY,
    const Color& color,
    std::vector<uint32_t>& framebuffer,
    int fbWidth, int fbHeight)
{
    const auto& alphaAtlas = font.getAtlasAlpha();
    int atlasW = font.getAtlasWidth();
    int atlasH = font.getAtlasHeight();
    if (alphaAtlas.empty()) return;

    uint8_t cr = static_cast<uint8_t>(std::clamp(color.r * 255.0f, 0.0f, 255.0f));
    uint8_t cg = static_cast<uint8_t>(std::clamp(color.g * 255.0f, 0.0f, 255.0f));
    uint8_t cb = static_cast<uint8_t>(std::clamp(color.b * 255.0f, 0.0f, 255.0f));
    uint8_t ca = static_cast<uint8_t>(std::clamp(color.a * 255.0f, 0.0f, 255.0f));

    float curX = static_cast<float>(posX);
    float curY = static_cast<float>(posY + font.getFontSize()); // Alignement sur la ligne de base

    for (char c : text) {
        if (c == '\n') {
            curX = static_cast<float>(posX);
            curY += font.getFontSize();
            continue;
        }

        GlyphQuad q;
        font.getBakedQuad(c, curX, curY, q);

        int w = q.x1 - q.x0;
        int h = q.y1 - q.y0;

        for (int dy = 0; dy < h; ++dy) {
            int screenY = q.y0 + dy;
            if (screenY < 0 || screenY >= fbHeight) continue;

            float v = q.t0 + (q.t1 - q.t0) * (static_cast<float>(dy) / h);
            int texY = static_cast<int>(v * atlasH);

            for (int dx = 0; dx < w; ++dx) {
                int screenX = q.x0 + dx;
                if (screenX < 0 || screenX >= fbWidth) continue;

                float u = q.s0 + (q.s1 - q.s0) * (static_cast<float>(dx) / w);
                int texX = static_cast<int>(u * atlasW);

                uint8_t alpha = alphaAtlas[texY * atlasW + texX];
                if (alpha == 0) continue;

                float finalAlpha = (alpha / 255.0f) * (ca / 255.0f);
                int dstIdx = screenY * fbWidth + screenX;
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

