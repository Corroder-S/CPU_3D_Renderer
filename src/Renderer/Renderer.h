#pragma once
#include <cstdint>
#include <vector>
#include <SDL3/SDL.h>

#include "Maths/Matrix4x4.h"
#include "../Asset/Mesh.h"
#include "../Asset/Texture.h"
#include "../Asset/Light.h"
#include "../Scene/Scene.h"

class Renderer
{
public:
    Renderer() = default;

    void init(uint32_t width, uint32_t height);
    void renderScene(const Scene& scene);
    void drawMesh(const Mesh& mesh, const Maths::Matrix4x4<>& MVP,
                  const Maths::Matrix4x4<>& model, const Texture& texture,
                  const Light& light);
    void present(SDL_Texture* texture, SDL_Renderer* sdlRenderer);
    void clearFrameBuffer(uint32_t color);
    void clearDepthBuffer();

    uint32_t* getFrameBuffer() { return m_framebuffer.data(); }
    float*    getDepthBuffer() { return m_zBuffer.data(); }
    uint32_t  getWidth()  const { return m_width; }
    uint32_t  getHeight() const { return m_height; }

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    std::vector<uint32_t> m_framebuffer;
    std::vector<float>    m_zBuffer;

    // Buffers de travail réutilisés chaque frame
    std::vector<Maths::Vec4f> m_clipPositions;
    std::vector<Maths::Vec3f> m_worldNormals;

    void onResize(int32_t width, int32_t height);
};
