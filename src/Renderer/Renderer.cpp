#include "Renderer.h"
#include "Clip.h"
#include "Draw.h"
#include "Pipeline.h"

#include <algorithm>
#include <limits>

void Renderer::init(uint32_t width, uint32_t height)
{
    onResize(width, height);
}

void Renderer::renderScene(const Scene& scene)
{
    auto view = scene.camera.getViewMatrix();
    auto proj = scene.camera.getProjMatrix((float)m_width / m_height);
    auto viewProj = proj * view;

    for (auto& obj : scene.objects)
    {
        if (!obj.mesh || !obj.texture) continue;

        auto model = obj.transform.getModelMatrix();
        auto MVP = viewProj * model;
        drawMesh(*obj.mesh, MVP, model, *obj.texture, scene.lights[0]);
    }
}

void Renderer::drawMesh(const Mesh& mesh, const Maths::Matrix4x4<>& MVP,
                         const Maths::Matrix4x4<>& model, const Texture& texture,
                         const Light& light)
{
    size_t vertexCount = mesh.vertices.size();

    m_clipPositions.resize(vertexCount);
    m_worldNormals.resize(vertexCount);

    // 1. Transform vertices
    for (size_t i = 0; i < vertexCount; ++i)
    {
        m_clipPositions[i] = MVP * Maths::Vec4f(mesh.vertices[i], 1.0f);
        m_worldNormals[i]  = model.TransformDirection(mesh.vertexNormals[i]);
    }

    // 2. Per face: clip, NDC, viewport, cull, rasterize
    for (auto& f : mesh.faces)
    {
        ClipVertex v0 = { m_clipPositions[f.v.x], mesh.uvs[f.uv.x], m_worldNormals[f.v.x] };
        ClipVertex v1 = { m_clipPositions[f.v.y], mesh.uvs[f.uv.y], m_worldNormals[f.v.y] };
        ClipVertex v2 = { m_clipPositions[f.v.z], mesh.uvs[f.uv.z], m_worldNormals[f.v.z] };

        ClipVertex clipped[6];
        int numTris = clipTriangleNear(v0, v1, v2, clipped);

        for (int i = 0; i < numTris; ++i)
        {
            const ClipVertex& tv0 = clipped[i * 3 + 0];
            const ClipVertex& tv1 = clipped[i * 3 + 1];
            const ClipVertex& tv2 = clipped[i * 3 + 2];

            Maths::Vec3f invW = { 1.0f / tv0.clip.w, 1.0f / tv1.clip.w, 1.0f / tv2.clip.w };

            Maths::Vec3f ndc0 = { tv0.clip.x * invW.x, tv0.clip.y * invW.x, tv0.clip.z * invW.x };
            Maths::Vec3f ndc1 = { tv1.clip.x * invW.y, tv1.clip.y * invW.y, tv1.clip.z * invW.y };
            Maths::Vec3f ndc2 = { tv2.clip.x * invW.z, tv2.clip.y * invW.z, tv2.clip.z * invW.z };

            Maths::Vec2i sc0 = viewport(ndc0.x, ndc0.y, m_width, m_height);
            Maths::Vec2i sc1 = viewport(ndc1.x, ndc1.y, m_width, m_height);
            Maths::Vec2i sc2 = viewport(ndc2.x, ndc2.y, m_width, m_height);

            Maths::Vec3f a = { (float)sc0.x, (float)sc0.y, ndc0.z };
            Maths::Vec3f b = { (float)sc1.x, (float)sc1.y, ndc1.z };
            Maths::Vec3f c = { (float)sc2.x, (float)sc2.y, ndc2.z };

            int area = edge(a, b, c);
            if (area >= 0) continue;

            drawTriangle(
                a, b, c,
                tv0.uv, tv1.uv, tv2.uv,
                invW,
                tv0.normal, tv1.normal, tv2.normal,
                m_framebuffer, m_zBuffer, texture, light
            );
        }
    }
}

void Renderer::present(SDL_Texture* texture, SDL_Renderer* sdlRenderer)
{
    SDL_UpdateTexture(texture, nullptr, m_framebuffer.data(), m_width * sizeof(uint32_t));
    SDL_RenderClear(sdlRenderer);
    SDL_RenderTexture(sdlRenderer, texture, nullptr, nullptr);
    SDL_RenderPresent(sdlRenderer);
}

void Renderer::clearFrameBuffer(uint32_t color)
{
    std::fill(m_framebuffer.begin(), m_framebuffer.end(), color);
}

void Renderer::clearDepthBuffer()
{
    std::fill(m_zBuffer.begin(), m_zBuffer.end(), 1e30f);
}

void Renderer::onResize(int32_t width, int32_t height)
{
    m_width = width;
    m_height = height;
    m_framebuffer.resize(width * height, 0xFF101018);
    m_zBuffer.resize(width * height, 1e30f);
}
