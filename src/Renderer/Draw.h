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



static inline float edge(Maths::Vec3f a, Maths::Vec3f b, Maths::Vec3f c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static inline float edge_dxdy(int a, int b)
{
    return (a - b);
}

inline Maths::Vec3f computeEdgeWeights(Maths::Vec3f a, Maths::Vec3f b, Maths::Vec3f c, Maths::Vec3f p) {
    return { edge(b, c, p), edge(c, a, p), edge(a, b, p) };
}

struct EdgeGradients { Maths::Vec3f dx, dy; };
inline EdgeGradients computeEdgeGradients(Maths::Vec3f a, Maths::Vec3f b, Maths::Vec3f c) {
    return {
                { b.y - c.y, c.y - a.y, a.y - b.y },   // dx
                { c.x - b.x, a.x - c.x, b.x - a.x }    // dy
    };
}



struct ScreenTriangle {
    Maths::Vec3f a, b, c;
    Maths::Vec2f uv0, uv1, uv2;
    Maths::Vec3f invW;
    Maths::Vec3f n0, n1, n2;
    Maths::Vec3f wp0, wp1, wp2;

    ShaderUniforms uniforms;
    float avgZ = 0.0f;
    bool writeDepth = true;
};


struct RasterContext {
    uint32_t* framebuffer;
    float*    zbuffer;
    uint32_t       width;
    uint32_t       height;
    Maths::Vec2i     renderMin;
    Maths::Vec2i     renderMax;
};

struct InterpVec3 {
    Maths::Vec3f row;  // { nx_row, ny_row, nz_row }
    Maths::Vec3f dx;   // { dnx_dx, dny_dx, dnz_dx }
    Maths::Vec3f dy;   // { dnx_dy, dny_dy, dnz_dy }
};


struct BBox2D {
    int minX, minY, maxX, maxY;

    bool empty() const { return minX > maxX || minY > maxY; }

    static BBox2D fromTriangle(Maths::Vec3f a, Maths::Vec3f b, Maths::Vec3f c, Maths::Vec2i clipMin, Maths::Vec2i clipMax) {
        return {
            std::max(clipMin.x, std::min({(int)a.x, (int)b.x, (int)c.x})),
            std::max(clipMin.y, std::min({(int)a.y, (int)b.y, (int)c.y})),
            std::min(clipMax.x - 1, std::max({(int)a.x, (int)b.x, (int)c.x})),
            std::min(clipMax.y - 1, std::max({(int)a.y, (int)b.y, (int)c.y}))
        };
    }
};

struct ScanlineState {
    __m256 w0, w1, w2;       // edge weights
    __m256 z;                 // depth
    __m256 uw, vw, iw;       // UVs + invW
    Maths::Vec3<__m256> normals;    // normal
    Maths::Vec3<__m256> wp;   // world pos
    __m256 xVec;

    // Pré-calculés pour le step
    __m256 dw0_8, dw1_8, dw2_8, dz_8, duw_8, dvw_8, diw_8;
    Maths::Vec3<__m256> dn_8, dwp_8;

    void stepX()
    {
        w0 = _mm256_add_ps(w0, dw0_8);
        w1 = _mm256_add_ps(w1, dw1_8);
        w2 = _mm256_add_ps(w2, dw2_8);
        z = _mm256_add_ps(z, dz_8);
        uw = _mm256_add_ps(uw, duw_8);
        vw = _mm256_add_ps(vw, dvw_8);
        iw = _mm256_add_ps(iw, diw_8);

        normals.x = _mm256_add_ps(normals.x , dn_8.x );
        normals.y = _mm256_add_ps(normals.y , dn_8.y );
        normals.z = _mm256_add_ps(normals.z , dn_8.z );

        wp.x = _mm256_add_ps(wp.x, dwp_8.x);
        wp.y = _mm256_add_ps(wp.y, dwp_8.y);
        wp.z = _mm256_add_ps(wp.z, dwp_8.z);

        xVec = _mm256_add_ps(xVec, _mm256_set1_ps(8.0f));
    }

    FragmentInput_8 buildFragment() const  // perspective-correct + build frag
    {
        __m256 inv_iw = _mm256_rcp_ps(iw);
        return {
                { _mm256_mul_ps(uw, inv_iw), _mm256_mul_ps(vw, inv_iw) },
                normals,
                { _mm256_mul_ps(wp.x, inv_iw), _mm256_mul_ps(wp.y, inv_iw), _mm256_mul_ps(wp.z, inv_iw) }
        };
    }
};


inline __m256 makeRow8(float base, float step) {
    // [base, base+step, base+2*step, ..., base+7*step]
    return _mm256_fmadd_ps(
        _mm256_set_ps(7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f, 0.f),
        _mm256_set1_ps(step),
        _mm256_set1_ps(base)
    );
}

struct TriangleSetup
{
    Maths::Vec3<__m256> camPos8;
    // Edge weights + gradients (for step and coverage test)
    InterpVec3 edges;  // .row = w_row, .dx = dw.dx, .dy = dw.dy (non normalisés)

    Maths::Vec3f depthData; // z_row, dz_dx, dz_dy
    Maths::Vec3f uw, vw, iw;
    InterpVec3  normal;
    InterpVec3  worldPos;

    ScanlineState initScanline() const // crée les __m256 via makeRow8
    {
        ScanlineState scan;
        scan.w0 = makeRow8(edges.row.x, edges.dx.x);
        scan.w1 = makeRow8(edges.row.y, edges.dx.y);
        scan.w2 = makeRow8(edges.row.z, edges.dx.z);
        scan.z = makeRow8(depthData.x, depthData.y);
        scan.uw = makeRow8(uw.x, uw.y);
        scan.vw = makeRow8(vw.x, vw.y);
        scan.iw = makeRow8(iw.x, iw.y);
        scan.normals.x = makeRow8(normal.row.x, normal.dx.x);
        scan.normals.y = makeRow8(normal.row.y, normal.dx.y);
        scan.normals.z = makeRow8(normal.row.z, normal.dx.z);
        scan.wp.x = makeRow8(worldPos.row.x, worldPos.dx.x);
        scan.wp.y = makeRow8(worldPos.row.y, worldPos.dx.y);
        scan.wp.z = makeRow8(worldPos.row.z, worldPos.dx.z);

        // Pré-calcul des steps pour stepX
        scan.dw0_8 = _mm256_set1_ps(edges.dx.x * 8.0f);
        scan.dw1_8 = _mm256_set1_ps(edges.dx.y * 8.0f);
        scan.dw2_8 = _mm256_set1_ps(edges.dx.z * 8.0f);
        scan.dz_8  = _mm256_set1_ps(depthData.y * 8.0f);
        scan.duw_8 = _mm256_set1_ps(uw.y * 8.0f);
        scan.dvw_8 = _mm256_set1_ps(vw.y * 8.0f);
        scan.diw_8 = _mm256_set1_ps(iw.y * 8.0f);

        scan.dn_8  = { _mm256_set1_ps(normal.dx.x * 8.f),
               _mm256_set1_ps(normal.dx.y * 8.f),
               _mm256_set1_ps(normal.dx.z * 8.f) };
        scan.dwp_8 = { _mm256_set1_ps(worldPos.dx.x * 8.f),
                       _mm256_set1_ps(worldPos.dx.y * 8.f),
                       _mm256_set1_ps(worldPos.dx.z * 8.f) };
        return scan;
    }
    void stepRow()
    {
        edges.row += edges.dy;  // avance tous les _row de dy
        depthData.x += depthData.z;
        uw.x += uw.z;
        vw.x += vw.z;
        iw.x += iw.z;
        normal.row += normal.dy;  // avance tous les _row de dy
        worldPos.row += worldPos.dy;  // avance tous les _row de dy
    }
};

inline TriangleSetup setupTriangle(const ScreenTriangle& tri, const BBox2D& bbox)
{
    TriangleSetup triangle;

    triangle.camPos8 = { _mm256_set1_ps(tri.uniforms.cameraPos.x), _mm256_set1_ps(tri.uniforms.cameraPos.y), _mm256_set1_ps(tri.uniforms.cameraPos.z) };
    EdgeGradients dw = computeEdgeGradients(tri.a, tri.b, tri.c);
    triangle.edges.dx = dw.dx;
    triangle.edges.dy = dw.dy;
    triangle.edges.row = { computeEdgeWeights(tri.a, tri.b, tri.c, { (float)bbox.minX + 0.5f, (float)bbox.minY + 0.5f, 0.0f }) };
    Maths::Vec3f w_row = triangle.edges.row;
    float area = w_row.x + w_row.y + w_row.z;
    float invArea = 1.0f / area;
    Maths::Vec3f fw_row = w_row * invArea;
    Maths::Vec3f zVerts = { tri.a.z, tri.b.z, tri.c.z };

    EdgeGradients dfw = { dw.dx * invArea, dw.dy * invArea };

    triangle.depthData.x =  Maths::Dot(fw_row, zVerts);
    triangle.depthData.y =  Maths::Dot(dfw.dx, zVerts);
    triangle.depthData.z =  Maths::Dot(dfw.dy, zVerts);

    float iw_row = Maths::Dot(fw_row, tri.invW);
    Maths::Vec3f uW = { tri.uv0.x * tri.invW.x, tri.uv1.x * tri.invW.y, tri.uv2.x * tri.invW.z };
    Maths::Vec3f vW = { tri.uv0.y * tri.invW.x, tri.uv1.y * tri.invW.y, tri.uv2.y * tri.invW.z };
    float uw_row = Maths::Dot(fw_row, uW);
    float vw_row = Maths::Dot(fw_row, vW);
    Maths::Vec2f duw = { Maths::Dot(dfw.dx, uW), Maths::Dot(dfw.dy, uW) };
    Maths::Vec2f dvw = { Maths::Dot(dfw.dx, vW), Maths::Dot(dfw.dy, vW) };
    Maths::Vec2f diw = { Maths::Dot(dfw.dx, tri.invW), Maths::Dot(dfw.dy, tri.invW) };
    triangle.uw = { uw_row, duw.x, duw.y };
    triangle.vw = { vw_row, dvw.x, dvw.y };
    triangle.iw = { iw_row, diw.x, diw.y };

    triangle.normal.row = {
        Maths::Dot(fw_row, { tri.n0.x, tri.n1.x, tri.n2.x }),
        Maths::Dot(fw_row, { tri.n0.y, tri.n1.y, tri.n2.y }),
        Maths::Dot(fw_row, { tri.n0.z, tri.n1.z, tri.n2.z })
    };
    triangle.normal.dx = {
        Maths::Dot(dfw.dx, { tri.n0.x, tri.n1.x, tri.n2.x }),
        Maths::Dot(dfw.dx, { tri.n0.y, tri.n1.y, tri.n2.y }),
        Maths::Dot(dfw.dx, { tri.n0.z, tri.n1.z, tri.n2.z })
    };
    triangle.normal.dy = {
        Maths::Dot(dfw.dy, { tri.n0.x, tri.n1.x, tri.n2.x }),
        Maths::Dot(dfw.dy, { tri.n0.y, tri.n1.y, tri.n2.y }),
        Maths::Dot(dfw.dy, { tri.n0.z, tri.n1.z, tri.n2.z })
    };

    Maths::Vec3f wpW0 = tri.wp0 * tri.invW.x;
    Maths::Vec3f wpW1 = tri.wp1 * tri.invW.y;
    Maths::Vec3f wpW2 = tri.wp2 * tri.invW.z;

    triangle.worldPos.row = {
        fw_row.x * wpW0.x + fw_row.y * wpW1.x + fw_row.z * wpW2.x,
        fw_row.x * wpW0.y + fw_row.y * wpW1.y + fw_row.z * wpW2.y,
        fw_row.x * wpW0.z + fw_row.y * wpW1.z + fw_row.z * wpW2.z
    };
    triangle.worldPos.dx = {
        dfw.dx.x * wpW0.x + dfw.dx.y * wpW1.x + dfw.dx.z * wpW2.x,
        dfw.dx.x * wpW0.y + dfw.dx.y * wpW1.y + dfw.dx.z * wpW2.y,
        dfw.dx.x * wpW0.z + dfw.dx.y * wpW1.z + dfw.dx.z * wpW2.z
    };
    triangle.worldPos.dy = {
        dfw.dy.x * wpW0.x + dfw.dy.y * wpW1.x + dfw.dy.z * wpW2.x,
        dfw.dy.x * wpW0.y + dfw.dy.y * wpW1.y + dfw.dy.z * wpW2.y,
        dfw.dy.x * wpW0.z + dfw.dy.y * wpW1.z + dfw.dy.z * wpW2.z
    };


    return triangle;
}


inline __m256 computeMask(const ScanlineState& scan, const RasterContext& ctx, int idx, int remaining)
{
    __m256 zero  = _mm256_setzero_ps();
    __m256 mask0 = _mm256_cmp_ps(scan.w0, zero, _CMP_GE_OS);
    __m256 mask1 = _mm256_cmp_ps(scan.w1, zero, _CMP_GE_OS);
    __m256 mask2 = _mm256_cmp_ps(scan.w2, zero, _CMP_GE_OS);
    __m256 triMask = _mm256_and_ps(_mm256_and_ps(mask0, mask1), mask2);

    // 2.8 — Depth test: pixel passes if its z <= current z-buffer value
    //if the pixel is closer to the camera than the current depth buffer value, draw it, otherwise discard it
    __m256 zbufferVec = _mm256_loadu_ps(&ctx.zbuffer[idx]);
    __m256 depthMask = _mm256_cmp_ps(scan.z, zbufferVec, _CMP_LE_OS);


    __m256 finalMask = _mm256_and_ps(triMask, depthMask);


    // 2.11 — End-of-row mask: disables lanes beyond remaining pixels
    if (remaining < 8) {
        __m256i boundsMask = _mm256_cmpgt_epi32(
            _mm256_set1_epi32(remaining),
            _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0)
        );
        finalMask = _mm256_and_ps(finalMask, _mm256_castsi256_ps(boundsMask));
    }
    return finalMask;
}

inline void alphaBlend_8(const Color8& src, __m256 mask, uint32_t* framebuffer, int idx)
{
    // 2.17 — Alpha blending (src over): load existing framebuffer pixels,
                //         blend dst = dst * (1 - alpha) + src * alpha per channel
    __m256i dst = _mm256_maskload_epi32((const int*)&framebuffer[idx], _mm256_castps_si256(mask));

    __m256 dstR = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(dst, 16), _mm256_set1_epi32(0xFF)));
    __m256 dstG = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(dst, 8), _mm256_set1_epi32(0xFF)));
    __m256 dstB = _mm256_cvtepi32_ps(_mm256_and_si256(dst, _mm256_set1_epi32(0xFF)));

    dstR = _mm256_mul_ps(dstR, _mm256_sub_ps(_mm256_set1_ps(1.0f), src.a));
    dstG = _mm256_mul_ps(dstG, _mm256_sub_ps(_mm256_set1_ps(1.0f), src.a));
    dstB = _mm256_mul_ps(dstB, _mm256_sub_ps(_mm256_set1_ps(1.0f), src.a));

    dstR = _mm256_add_ps(dstR, _mm256_mul_ps(_mm256_mul_ps(src.r, _mm256_set1_ps(255.0f)), src.a));
    dstG = _mm256_add_ps(dstG, _mm256_mul_ps(_mm256_mul_ps(src.g, _mm256_set1_ps(255.0f)), src.a));
    dstB = _mm256_add_ps(dstB, _mm256_mul_ps(_mm256_mul_ps(src.b, _mm256_set1_ps(255.0f)), src.a));

                // 2.18 — Pack ARGB channels into uint32 and masked write to framebuffer
    __m256i ri = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(dstR, _mm256_setzero_ps()), _mm256_set1_ps(255.0f)));
    __m256i gi = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(dstG, _mm256_setzero_ps()), _mm256_set1_ps(255.0f)));
    __m256i bi = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(dstB, _mm256_setzero_ps()), _mm256_set1_ps(255.0f)));
    __m256i ai = _mm256_cvtps_epi32(_mm256_min_ps(_mm256_max_ps(_mm256_mul_ps(src.a, _mm256_set1_ps(255.0f)), _mm256_setzero_ps()), _mm256_set1_ps(255.0f)));

    __m256i packed = _mm256_or_si256(
    _mm256_or_si256(_mm256_slli_epi32(ai, 24), _mm256_slli_epi32(ri, 16)),
    _mm256_or_si256(_mm256_slli_epi32(gi, 8), bi)
    );

    _mm256_maskstore_epi32((int*)&framebuffer[idx], _mm256_castps_si256(mask), packed);
}

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





static inline void drawTriangle(const ScreenTriangle& tri, const RasterContext& ctx)
{
    BBox2D bbox = BBox2D::fromTriangle(tri.a, tri.b, tri.c, ctx.renderMin, ctx.renderMax);
    int covered = ((bbox.maxX - bbox.minX) / 8 + 1) * 8;

    TriangleSetup setup = setupTriangle(tri, bbox);     // ~1 ligne → tout le scalar setup

    int idx = bbox.minY * ctx.width + bbox.minX;
    __m256 maxXV = _mm256_set1_ps((float)bbox.maxX + 0.5f);

    for (int y = bbox.minY; y <= bbox.maxY; y++)
    {
        ScanlineState scan = setup.initScanline();      // makeRow8 pour tous les attributs

        for (int x = bbox.minX; x <= bbox.maxX; x += 8)
        {
            __m256 mask = computeMask(scan, ctx, idx, bbox.maxX - x + 1);

            if (!_mm256_testz_ps(mask, mask))
            {
                if (tri.writeDepth)
                    _mm256_maskstore_ps(&ctx.zbuffer[idx], _mm256_castps_si256(mask), scan.z);

                FragmentInput_8 frag = scan.buildFragment();
                Color8 color = evaluateBlinnPhong_8(frag, tri.uniforms, setup.camPos8);
                alphaBlend_8(color, mask, ctx.framebuffer, idx);
            }

            scan.stepX();
            idx += 8;
        }

        idx += ctx.width - covered;
        setup.stepRow();
    }
}



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

