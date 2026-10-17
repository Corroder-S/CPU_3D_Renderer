#pragma once

#include "Framebuffer.h"
#include "Maths/Vec2.h"
#include "Maths/Vec3.h"
#include "Maths/Vec4.h"
#include "Maths/Matrix4x4.h"

static inline Maths::Vec2i viewport(float ndcX, float ndcY, int width, int height)
{
    return Maths::Vec2i(
        (int)((ndcX + 1.0f) * 0.5f * width),
        (int)((1.0f - (ndcY + 1.0f) * 0.5f) * height));
}

static inline void projectVertex(const Maths::Matrix4x4<>& mvp, const Maths::Vec3<> vertex, Maths::Vec2i& out, float& depth, float& invW)
{
    Maths::Vec4<> clip = mvp * Maths::Vec4<>(vertex, 1.0f);
    Maths::Vec3<> ndc  = { clip.x / clip.w, clip.y / clip.w, clip.z / clip.w };
    out   = viewport(ndc.x, ndc.y, RENDER_WIDTH, RENDER_HEIGHT);
    depth = ndc.z;
    invW  = 1.0f / clip.w;
}

static inline void projectMesh(const Maths::Matrix4x4<>& mvp, const Maths::Vec3<>* vertices, int vertexCount, Maths::Vec2i* out, float* depths, float* invWs)
{
    for (int i = 0; i < vertexCount; i++)
        projectVertex(mvp, vertices[i], out[i], depths[i], invWs[i]);

}
