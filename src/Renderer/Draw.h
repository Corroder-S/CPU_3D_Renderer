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

static inline float edge(Maths::Vec3f a, Maths::Vec3f b, Maths::Vec3f c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static inline float edge_dxdy(int a, int b)
{
    return (a - b);
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

    Maths::Vec3<__m256> L8 = { _mm256_set1_ps(light.direction.x), _mm256_set1_ps(light.direction.y), _mm256_set1_ps(light.direction.z) };
    Maths::Vec3<__m256> C8 = { _mm256_set1_ps(uniforms.cameraPos.x), _mm256_set1_ps(uniforms.cameraPos.y), _mm256_set1_ps(uniforms.cameraPos.z) };

    int minX = std::max(renderMin.x, std::min({ (int)a.x, (int)b.x, (int)c.x }));
    int maxX = std::min(renderMax.x - 1, std::max({ (int)a.x, (int)b.x, (int)c.x }));
    int minY = std::max(renderMin.y, std::min({ (int)a.y, (int)b.y, (int)c.y }));
    int maxY = std::min(renderMax.y - 1, std::max({ (int)a.y, (int)b.y, (int)c.y }));

    Maths::Vec3f minPos = { (float)minX, (float)minY, 0.0f };

    int covered = ((maxX - minX) / 8 + 1) * 8;


    float w0_row = edge(b, c, minPos);
    float w1_row = edge(c, a, minPos);
    float w2_row = edge(a, b, minPos);

    float area = w0_row + w1_row + w2_row;
    float invArea = 1.0f / area;

    float dw0_dx = edge_dxdy(b.y, c.y);
    float dw0_dy = edge_dxdy(c.x, b.x);
    float dw1_dx = edge_dxdy(c.y, a.y);
    float dw1_dy = edge_dxdy(a.x, c.x);
    float dw2_dx = edge_dxdy(a.y, b.y);
    float dw2_dy = edge_dxdy(b.x, a.x);

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
    for (uint32_t y = minY; y <= maxY; y++)
    {

        __m256 w0  = _mm256_set_ps(w0_row + 7*dw0_dx, w0_row + 6*dw0_dx, w0_row + 5*dw0_dx, w0_row + 4*dw0_dx, w0_row + 3*dw0_dx, w0_row + 2*dw0_dx, w0_row + 1*dw0_dx, w0_row);
        __m256 w1  = _mm256_set_ps(w1_row + 7*dw1_dx, w1_row + 6*dw1_dx, w1_row + 5*dw1_dx, w1_row + 4*dw1_dx, w1_row + 3*dw1_dx, w1_row + 2*dw1_dx, w1_row + 1*dw1_dx, w1_row);
        __m256 w2  = _mm256_set_ps(w2_row + 7*dw2_dx, w2_row + 6*dw2_dx, w2_row + 5*dw2_dx, w2_row + 4*dw2_dx, w2_row + 3*dw2_dx, w2_row + 2*dw2_dx, w2_row + 1*dw2_dx, w2_row);

        __m256 uw  = _mm256_set_ps(uw_row + 7*duw_dx, uw_row + 6*duw_dx, uw_row + 5*duw_dx, uw_row + 4*duw_dx, uw_row + 3*duw_dx, uw_row + 2*duw_dx, uw_row + 1*duw_dx, uw_row);
        __m256 vw  = _mm256_set_ps(vw_row + 7*dvw_dx, vw_row + 6*dvw_dx, vw_row + 5*dvw_dx, vw_row + 4*dvw_dx, vw_row + 3*dvw_dx, vw_row + 2*dvw_dx, vw_row + 1*dvw_dx, vw_row);
        __m256 iw  = _mm256_set_ps(iw_row + 7*diw_dx, iw_row + 6*diw_dx, iw_row + 5*diw_dx, iw_row + 4*diw_dx, iw_row + 3*diw_dx, iw_row + 2*diw_dx, iw_row + 1*diw_dx, iw_row);

        __m256 z   = _mm256_set_ps(z_row + 7*dz_dx, z_row + 6*dz_dx, z_row + 5*dz_dx, z_row + 4*dz_dx, z_row + 3*dz_dx, z_row + 2*dz_dx, z_row + 1*dz_dx, z_row);

        __m256 nx  = _mm256_set_ps(nx_row + 7*dnx_dx, nx_row + 6*dnx_dx, nx_row + 5*dnx_dx, nx_row + 4*dnx_dx, nx_row + 3*dnx_dx, nx_row + 2*dnx_dx, nx_row + 1*dnx_dx, nx_row);
        __m256 ny  = _mm256_set_ps(ny_row + 7*dny_dx, ny_row + 6*dny_dx, ny_row + 5*dny_dx, ny_row + 4*dny_dx, ny_row + 3*dny_dx, ny_row + 2*dny_dx, ny_row + 1*dny_dx, ny_row);
        __m256 nz  = _mm256_set_ps(nz_row + 7*dnz_dx, nz_row + 6*dnz_dx, nz_row + 5*dnz_dx, nz_row + 4*dnz_dx, nz_row + 3*dnz_dx, nz_row + 2*dnz_dx, nz_row + 1*dnz_dx, nz_row);

        __m256 pxw = _mm256_set_ps(pxw_row + 7*dpxw_dx, pxw_row + 6*dpxw_dx, pxw_row + 5*dpxw_dx, pxw_row + 4*dpxw_dx, pxw_row + 3*dpxw_dx, pxw_row + 2*dpxw_dx, pxw_row + 1*dpxw_dx, pxw_row);
        __m256 pyw = _mm256_set_ps(pyw_row + 7*dpyw_dx, pyw_row + 6*dpyw_dx, pyw_row + 5*dpyw_dx, pyw_row + 4*dpyw_dx, pyw_row + 3*dpyw_dx, pyw_row + 2*dpyw_dx, pyw_row + 1*dpyw_dx, pyw_row);
        __m256 pzw = _mm256_set_ps(pzw_row + 7*dpzw_dx, pzw_row + 6*dpzw_dx, pzw_row + 5*dpzw_dx, pzw_row + 4*dpzw_dx, pzw_row + 3*dpzw_dx, pzw_row + 2*dpzw_dx, pzw_row + 1*dpzw_dx, pzw_row);



        for (uint32_t x = minX; x <= maxX; x+=8)
        {
            __m256 orResult = _mm256_or_ps(_mm256_or_ps(w0, w1), w2);
            __m256 triMask = _mm256_cmp_ps(orResult, _mm256_setzero_ps(), _CMP_GE_OS);

            __m256 zbufferVec = _mm256_loadu_ps(&zbuffer[idx]);
            __m256 depthMask = _mm256_cmp_ps(z, zbufferVec, _CMP_LE_OS);

            __m256 finalMask = _mm256_and_ps(triMask, depthMask);



            if (!_mm256_testz_ps(finalMask, finalMask))
            {
                if (writeDepth)
                {
                    _mm256_maskstore_ps(&zbuffer[idx], _mm256_castps_si256(finalMask), z);
                }

                __m256 inv_iw = _mm256_rcp_ps(iw);

                __m256 u = _mm256_mul_ps(uw, inv_iw);
                __m256 v = _mm256_mul_ps(vw, inv_iw);
                __m256 worldPosX = _mm256_mul_ps(pxw, inv_iw);
                __m256 worldPosY = _mm256_mul_ps(pyw, inv_iw);
                __m256 worldPosZ = _mm256_mul_ps(pzw, inv_iw);

                FragmentInput_8 frag;

                frag.uv = { u, v };
                frag.normal = { nx, ny, nz };
                frag.worldPos = { worldPosX, worldPosY, worldPosZ };



                Color8 color = evaluateBlinnPhong_8(frag, uniforms, L8, C8);


                __m256i dst = _mm256_loadu_si256((__m256i*)&framebuffer[idx]);

                __m256 dstR = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(dst, 16), _mm256_set1_epi32(0xFF)));
                __m256 dstG = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(dst, 8), _mm256_set1_epi32(0xFF)));
                __m256 dstB = _mm256_cvtepi32_ps(_mm256_and_si256(dst, _mm256_set1_epi32(0xFF)));

                dstR = _mm256_mul_ps(dstR, _mm256_sub_ps(_mm256_set1_ps(1.0f), color.a));
                dstG = _mm256_mul_ps(dstG, _mm256_sub_ps(_mm256_set1_ps(1.0f), color.a));
                dstB = _mm256_mul_ps(dstB, _mm256_sub_ps(_mm256_set1_ps(1.0f), color.a));

                dstR = _mm256_add_ps(dstR, _mm256_mul_ps(_mm256_mul_ps(color.r, _mm256_set1_ps(255.0f)), color.a));
                dstG = _mm256_add_ps(dstG, _mm256_mul_ps(_mm256_mul_ps(color.g, _mm256_set1_ps(255.0f)), color.a));
                dstB = _mm256_add_ps(dstB, _mm256_mul_ps(_mm256_mul_ps(color.b, _mm256_set1_ps(255.0f)), color.a));


                __m256i ri = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(dstR, _mm256_setzero_ps()), _mm256_set1_ps(255.0f)));
                __m256i gi = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(dstG, _mm256_setzero_ps()), _mm256_set1_ps(255.0f)));
                __m256i bi = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(dstB, _mm256_setzero_ps()), _mm256_set1_ps(255.0f)));
                __m256i ai = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(_mm256_mul_ps(color.a, _mm256_set1_ps(255.0f)), _mm256_setzero_ps()), _mm256_set1_ps(255.0f)));

                __m256i packed = _mm256_or_si256(
                    _mm256_or_si256(_mm256_slli_epi32(ai, 24), _mm256_slli_epi32(ri, 16)),
                    _mm256_or_si256(_mm256_slli_epi32(gi, 8), bi)
                );

                _mm256_maskstore_epi32((int*)&framebuffer[idx], _mm256_castps_si256(finalMask), packed);
            }

            w0 = _mm256_add_ps(w0, _mm256_set1_ps(dw0_dx*8));
            w1 = _mm256_add_ps(w1, _mm256_set1_ps(dw1_dx*8));
            w2 = _mm256_add_ps(w2, _mm256_set1_ps(dw2_dx*8));
            z = _mm256_add_ps(z, _mm256_set1_ps(dz_dx*8));
            uw = _mm256_add_ps(uw, _mm256_set1_ps(duw_dx*8));
            vw = _mm256_add_ps(vw, _mm256_set1_ps(dvw_dx*8));
            iw = _mm256_add_ps(iw, _mm256_set1_ps(diw_dx*8));
            nx = _mm256_add_ps(nx, _mm256_set1_ps(dnx_dx*8));
            ny = _mm256_add_ps(ny, _mm256_set1_ps(dny_dx*8));
            nz = _mm256_add_ps(nz, _mm256_set1_ps(dnz_dx*8));
            pxw = _mm256_add_ps(pxw, _mm256_set1_ps(dpxw_dx*8));
            pyw = _mm256_add_ps(pyw, _mm256_set1_ps(dpyw_dx*8));
            pzw = _mm256_add_ps(pzw, _mm256_set1_ps(dpzw_dx*8));

            idx+= 8;
        }

        idx += RENDER_WIDTH - covered;
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
    uint32_t posX, uint32_t posY,
    const Color& color,
    std::vector<uint32_t>& framebuffer,
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
    float curY = static_cast<float>(posY + font.getFontSize()); // Alignement sur la ligne de base

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
            if (screenY < 0 || screenY >= fbHeight) continue;

            float v = q.t0 + (q.t1 - q.t0) * (static_cast<float>(dy) / h);
            uint32_t texY = static_cast<uint32_t>(v * atlasH);

            for (uint32_t dx = 0; dx < w; ++dx) {
                uint32_t screenX = q.x0 + dx;
                if (screenX < 0 || screenX >= fbWidth) continue;

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

