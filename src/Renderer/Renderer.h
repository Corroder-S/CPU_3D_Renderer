#pragma once
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <SDL3/SDL.h>

#include "Maths/Matrix4x4.h"
#include "Asset/Mesh.h"
#include "Asset/Material.h"
#include "Asset/Light.h"
#include "Scene/Scene.h"
#include "Clip.h"
#include "Draw.h"
#include "Pipeline.h"

struct Worker {
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    std::condition_variable finished;
    std::atomic<bool> hasWork = false;
    std::atomic<bool> stop = false;
    Maths::Vec2i tileMin;
    Maths::Vec2i tileMax;
};

struct ScreenTriangle {
    Maths::Vec3f a, b, c;
    Maths::Vec2f uv0, uv1, uv2;
    Maths::Vec3f invW;
    Maths::Vec3f n0, n1, n2;
    Maths::Vec3f wp0, wp1, wp2;

    ShaderUniforms uniforms; 
    const Light* light = nullptr;
    float avgZ = 0.0f;
    bool writeDepth = true;
};

class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    void init(uint32_t width, uint32_t height);
    void renderScene(const Scene& scene);
    void drawMesh(const Mesh& mesh, const Maths::Matrix4x4<>& MVP,
        const Maths::Matrix4x4<>& model, const ShaderUniforms& uniforms,
        const Light& light, bool isTransparent, std::vector<ScreenTriangle>& outTriangles);
    void drawText(const Font& font, const std::string& text, int x, int y, const Color& color);
    void drawParticle(const ParticleVertex &particle, const Maths::Matrix4x4<>& viewProj);
    void present(SDL_Texture* texture, SDL_Renderer* sdlRenderer);
    void clearFrameBuffer(uint32_t color);
    void clearDepthBuffer();

    uint32_t* getFrameBuffer() { return m_framebuffer.data(); }
    float* getDepthBuffer() { return m_zBuffer.data(); }
    uint32_t  getWidth()  const { return m_width; }
    uint32_t  getHeight() const { return m_height; }

    void setParticleManager(ParticleManager* manager) { m_particleManager = manager; }
private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    std::vector<uint32_t> m_framebuffer;
    std::vector<float>    m_zBuffer;

    Worker m_workers[4];

    std::vector<Maths::Vec4f> m_clipPositions;
    std::vector<Maths::Vec3f> m_worldNormals;
    std::vector<Maths::Vec3f> m_worldPositions;
    std::vector<ScreenTriangle> m_screenTriangles;

    std::vector<ScreenTriangle> m_opaqueTriangles;
    std::vector<ScreenTriangle> m_transparentTriangles;

    ParticleManager* m_particleManager;

    void onResize(int32_t width, int32_t height);
    void workerLoop(Worker& w);
};