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
#include "Scene/Camera.h"
#include "Scene/Transform.h"
#include "Asset/Font.h"
#include "Renderer/Emitter/ParticleManager.h"
#include "Clip.h"
#include "Draw.h"
#include "Pipeline.h"

struct Worker {
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    std::condition_variable finished;
    bool hasWork = false;
    bool stop = false;
};

struct Tile {
    int x_min, y_min, x_max, y_max;
};




class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    void init(uint32_t width, uint32_t height);

    void drawSubMesh(const Mesh& mesh, const SubMesh& submesh,
        const Maths::Matrix4x4<>& model, const ShaderUniforms& uniforms,
        bool isTransparent, std::vector<ScreenTriangle>& outTriangles);

    void drawMesh(const Mesh& mesh, const Maths::Matrix4x4<>& MVP,
        const Maths::Matrix4x4<>& model, const ShaderUniforms& uniforms,
        bool isTransparent, std::vector<ScreenTriangle>& outTriangles);

    void beginFrame(const Camera& cam, float aspectRatio);

    void setLights(const Light* lights, uint32_t count);

    void submitMesh(const Mesh& mesh, const Transform& transform, const std::vector<Material>& materials);

    void submitMesh(const Mesh& mesh, const Transform& transform);

    void submitMesh(const Mesh& mesh, const Transform& transform, const Material& material);

    void endFrame();

    void drawText(const Font& font, const std::string& text, int x, int y, const Color& color);
    void drawParticle(const ParticleVertex& particle, const Maths::Matrix4x4<>& viewProj);
    void present(SDL_Window* window);
    void clearFrameBuffer(uint32_t color);
    void clearDepthBuffer();

    uint32_t* getFrameBuffer() { return m_framebuffer; }
    float* getDepthBuffer() { return m_zBuffer.data(); }
    uint32_t  getWidth()  const { return m_width; }
    uint32_t  getHeight() const { return m_height; }

    void setFrameBuffer(uint32_t* framebuffer) { m_framebuffer = framebuffer; }
    void setParticleManager(ParticleManager* manager) { m_particleManager = manager; }

    SDL_Surface* getSDLSURFACE() { return m_sdlSurface; }
    void setSDLSURFACE(SDL_Surface* surface) { m_sdlSurface = surface; }
    void onResize(uint32_t width, uint32_t height);

    double getTimeSceneMs()     const { return m_timeSceneMs; }
    double getTimeClearBinsMs() const { return m_timeClearBinsMs; }
    double getTimeMeshMs()      const { return m_timeMeshMs; }
    double getTimeBinMs()       const { return m_timeBinMs; }
    double getTimeRasterMs()    const { return m_timeRasterMs; }

private:
    SDL_Surface* m_sdlSurface = nullptr;
    double m_timeSceneMs = 0.0;
    double m_timeClearBinsMs = 0.0;
    double m_timeMeshMs = 0.0;
    double m_timeBinMs = 0.0;
    double m_timeRasterMs = 0.0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    uint32_t* m_framebuffer = nullptr;
    std::vector<float>    m_zBuffer;

    std::vector<Maths::Vec4f> m_clipPositions;
    std::vector<Maths::Vec3f> m_worldPositions;
    std::vector<ScreenTriangle> m_screenTriangles;

    std::vector<ScreenTriangle> m_opaqueTriangles;
    std::vector<ScreenTriangle> m_transparentTriangles;

    ParticleManager* m_particleManager = nullptr;

    uint32_t m_numWorkers = 0;
    std::vector<std::unique_ptr<Worker>> m_workers;

    std::vector<std::vector<ScreenTriangle>> m_tileBins;
    std::atomic<uint32_t> m_tileCounter{ 0 };
    std::vector<Tile> m_tiles;
    const int TILE_SIZE = 160;
    Maths::Matrix4x4<> m_viewProj;
    Maths::Vec3f m_camPosition;
    const Light* m_frameLights = nullptr;
    uint32_t m_frameLightCount = 0;

    void initTiles();
    void workerLoop(Worker& w);
};