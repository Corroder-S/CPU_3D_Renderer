#include "Renderer.h"
#include <algorithm>
#include <thread>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;
static double toMs(Clock::time_point a, Clock::time_point b)
{
    return std::chrono::duration<double, std::milli>(b - a).count();
}

Renderer::~Renderer()
{
    for (int i = 0; i < m_numWorkers; i++) {
        m_workers[i]->stop = true;
        m_workers[i]->cv.notify_one();
        m_workers[i]->thread.join();
    }
}

void Renderer::init(uint32_t width, uint32_t height)
{
    onResize(width, height);

    m_numWorkers = std::max(1, (int)std::thread::hardware_concurrency() - 1);
    m_workers.resize(m_numWorkers);

    for (int i = 0; i < m_numWorkers; i++) {
        m_workers[i] = std::make_unique<Worker>();
        m_workers[i]->thread = std::thread(&Renderer::workerLoop, this, std::ref(*m_workers[i]));
    }
}

void Renderer::drawSubMesh(const Mesh& mesh, const SubMesh& submesh,
    const Maths::Matrix4x4<>& model, const ShaderUniforms& uniforms,
    bool isTransparent, std::vector<ScreenTriangle>& outTriangles)
{
    for (size_t fi = 0; fi < submesh.faces.size(); ++fi)
    {
        const auto& f = submesh.faces[fi];

        const Maths::Vec3f n0 = Maths::Normalize(model.TransformDirection(submesh.cornerNormals[fi * 3 + 0]));
        const Maths::Vec3f n1 = Maths::Normalize(model.TransformDirection(submesh.cornerNormals[fi * 3 + 1]));
        const Maths::Vec3f n2 = Maths::Normalize(model.TransformDirection(submesh.cornerNormals[fi * 3 + 2]));

        ClipVertex v0 = { m_clipPositions[f.v.x], mesh.uvs[f.uv.x], n0, m_worldPositions[f.v.x] };
        ClipVertex v1 = { m_clipPositions[f.v.z], mesh.uvs[f.uv.z], n1, m_worldPositions[f.v.z] };
        ClipVertex v2 = { m_clipPositions[f.v.y], mesh.uvs[f.uv.y], n2, m_worldPositions[f.v.y] };

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
            if (area <= 0) continue;

            ScreenTriangle tri;
            tri.a = a; tri.b = b; tri.c = c;
            tri.uv0 = tv0.uv; tri.uv1 = tv1.uv; tri.uv2 = tv2.uv;
            tri.invW = invW;
            tri.n0 = tv0.normal; tri.n1 = tv1.normal; tri.n2 = tv2.normal;
            tri.wp0 = tv0.worldPos; tri.wp1 = tv1.worldPos; tri.wp2 = tv2.worldPos;
            tri.uniforms = uniforms;

            tri.avgZ = (a.z + b.z + c.z) / 3.0f;
            tri.writeDepth = !isTransparent;

            outTriangles.push_back(tri);
        }
    }
}

void Renderer::drawMesh(const Mesh& mesh, const Maths::Matrix4x4<>& MVP, const Maths::Matrix4x4<>& model,
    const ShaderUniforms& uniforms, bool isTransparent, std::vector<ScreenTriangle>& outTriangles)
{
    const size_t vertexCount = mesh.vertices.size();
    m_clipPositions.resize(vertexCount);
    m_worldPositions.resize(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i)
    {
        m_clipPositions[i] = MVP * Maths::Vec4f(mesh.vertices[i], 1.0f);
        Maths::Vec4f wPos = model * Maths::Vec4f(mesh.vertices[i], 1.0f);
        m_worldPositions[i] = { wPos.x, wPos.y, wPos.z };
    }

    for (const auto& submesh : mesh.submeshes)
    {
        drawSubMesh(mesh, submesh, model, uniforms, isTransparent, outTriangles);
    }
}

void Renderer::beginFrame(const Camera& cam, float aspectRatio)
{
    m_timeMeshMs = 0.0;

    auto t0 = Clock::now();

    m_screenTriangles.clear();
    m_opaqueTriangles.clear();
    m_transparentTriangles.clear();
    for (auto& bin : m_tileBins)
        bin.clear();

    auto t1 = Clock::now();
    m_timeClearBinsMs = toMs(t0, t1);

    auto view = cam.getViewMatrix();
    auto proj = cam.getProjMatrix(aspectRatio);
    m_viewProj = proj * view;
    m_camPosition = cam.position;
}

void Renderer::submitMesh(const Mesh& mesh, const Transform& transform)
{
    auto t0 = Clock::now();

    auto model = transform.getModelMatrix();
    auto MVP = m_viewProj * model;

    const size_t vertexCount = mesh.vertices.size();
    m_clipPositions.resize(vertexCount);
    m_worldPositions.resize(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i)
    {
        m_clipPositions[i] = MVP * Maths::Vec4f(mesh.vertices[i], 1.0f);
        Maths::Vec4f wPos = model * Maths::Vec4f(mesh.vertices[i], 1.0f);
        m_worldPositions[i] = { wPos.x, wPos.y, wPos.z };
    }

    for (const auto& submesh : mesh.submeshes)
    {
        ShaderUniforms uniforms{ &submesh.material, m_frameLights, m_frameLightCount, m_camPosition };
        bool isTransparent = (submesh.material.dissolve < 1.0f);
        auto& targetList = isTransparent ? m_transparentTriangles : m_opaqueTriangles;

        drawSubMesh(mesh, submesh, model, uniforms, isTransparent, targetList);
    }

    m_timeMeshMs += toMs(t0, Clock::now());
}

void Renderer::submitMesh(const Mesh& mesh, const Transform& transform, const Material& overrideMaterial)
{
    auto t0 = Clock::now();

    auto model = transform.getModelMatrix();
    auto MVP = m_viewProj * model;

    const size_t vertexCount = mesh.vertices.size();
    m_clipPositions.resize(vertexCount);
    m_worldPositions.resize(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i)
    {
        m_clipPositions[i] = MVP * Maths::Vec4f(mesh.vertices[i], 1.0f);
        Maths::Vec4f wPos = model * Maths::Vec4f(mesh.vertices[i], 1.0f);
        m_worldPositions[i] = { wPos.x, wPos.y, wPos.z };
    }

    ShaderUniforms uniforms{ &overrideMaterial, m_frameLights, m_frameLightCount, m_camPosition };
    bool isTransparent = (overrideMaterial.dissolve < 1.0f);
    auto& targetList = isTransparent ? m_transparentTriangles : m_opaqueTriangles;

    for (const auto& submesh : mesh.submeshes)
    {
        drawSubMesh(mesh, submesh, model, uniforms, isTransparent, targetList);
    }

    m_timeMeshMs += toMs(t0, Clock::now());
}

void Renderer::submitMesh(const Mesh& mesh, const Transform& transform, const std::vector<Material>& materials)
{
    auto t0 = Clock::now();

    auto model = transform.getModelMatrix();
    auto MVP = m_viewProj * model;

    const size_t vertexCount = mesh.vertices.size();
    m_clipPositions.resize(vertexCount);
    m_worldPositions.resize(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i)
    {
        m_clipPositions[i] = MVP * Maths::Vec4f(mesh.vertices[i], 1.0f);
        Maths::Vec4f wPos = model * Maths::Vec4f(mesh.vertices[i], 1.0f);
        m_worldPositions[i] = { wPos.x, wPos.y, wPos.z };
    }

    for (size_t i = 0; i < mesh.submeshes.size(); ++i)
    {
        const auto& submesh = mesh.submeshes[i];
        const Material& mat = (i < materials.size()) ? materials[i] : submesh.material;

        ShaderUniforms uniforms{ &mat, m_frameLights, m_frameLightCount, m_camPosition };
        bool isTransparent = (mat.dissolve < 1.0f);
        auto& targetList = isTransparent ? m_transparentTriangles : m_opaqueTriangles;

        drawSubMesh(mesh, submesh, model, uniforms, isTransparent, targetList);
    }

    m_timeMeshMs += toMs(t0, Clock::now());
}

void Renderer::endFrame()
{
    auto tBin0 = Clock::now();

    if (!m_opaqueTriangles.empty())
    {
        for (auto& tri : m_opaqueTriangles)
        {
            int minX = (int)std::floor(std::min({ tri.a.x, tri.b.x, tri.c.x }));
            int maxX = (int)std::ceil(std::max({ tri.a.x, tri.b.x, tri.c.x }));
            int minY = (int)std::floor(std::min({ tri.a.y, tri.b.y, tri.c.y }));
            int maxY = (int)std::ceil(std::max({ tri.a.y, tri.b.y, tri.c.y }));

            for (int i = 0; i < (int)m_tiles.size(); i++)
            {
                auto& tile = m_tiles[i];
                if (maxX >= tile.x_min && minX < tile.x_max &&
                    maxY >= tile.y_min && minY < tile.y_max)
                {
                    m_tileBins[i].push_back(tri);
                }
            }
        }
    }

    if (!m_transparentTriangles.empty())
    {
        std::sort(m_transparentTriangles.begin(), m_transparentTriangles.end(),
            [](const ScreenTriangle& t1, const ScreenTriangle& t2) {
                return t1.avgZ > t2.avgZ;
            });

        for (auto& tri : m_transparentTriangles)
        {
            int minX = (int)std::floor(std::min({ tri.a.x, tri.b.x, tri.c.x }));
            int maxX = (int)std::ceil(std::max({ tri.a.x, tri.b.x, tri.c.x }));
            int minY = (int)std::floor(std::min({ tri.a.y, tri.b.y, tri.c.y }));
            int maxY = (int)std::ceil(std::max({ tri.a.y, tri.b.y, tri.c.y }));

            for (int i = 0; i < (int)m_tiles.size(); i++)
            {
                auto& tile = m_tiles[i];
                if (maxX >= tile.x_min && minX < tile.x_max &&
                    maxY >= tile.y_min && minY < tile.y_max)
                {
                    m_tileBins[i].push_back(tri);
                }
            }
        }
    }

    auto tBin1 = Clock::now();
    m_timeBinMs = toMs(tBin0, tBin1);

    auto tRaster0 = Clock::now();
    m_tileCounter = 0;
    for (int i = 0; i < m_numWorkers; i++) {
        std::lock_guard<std::mutex> lock(m_workers[i]->mutex);
        m_workers[i]->hasWork = true;
        m_workers[i]->cv.notify_one();
    }

    for (int i = 0; i < m_numWorkers; i++) {
        std::unique_lock<std::mutex> lock(m_workers[i]->mutex);
        m_workers[i]->finished.wait(lock, [&] { return !m_workers[i]->hasWork; });
    }
    auto tRaster1 = Clock::now();
    m_timeRasterMs = toMs(tRaster0, tRaster1);

    m_timeSceneMs = m_timeMeshMs + m_timeBinMs + m_timeRasterMs;
}

void Renderer::setLights(const Light* lights, uint32_t count)
{
    m_frameLights = lights;
    m_frameLightCount = count;
}

void Renderer::drawText(const Font& font, const std::string& text, int x, int y, const Color& color)
{
    ::drawText(font, text, x, y, color, m_framebuffer, m_width, m_height);
}

void Renderer::drawParticle(const ParticleVertex& particle, const Maths::Matrix4x4<>& viewProj)
{
    Maths::Vec4f clipped = { particle.position.x, particle.position.y, particle.position.z, 1.0f };
    clipped = viewProj * clipped;
    if (clipped.w > 0.0f)
    {
        float ndcX = clipped.x / clipped.w;
        float ndcY = clipped.y / clipped.w;
        float z = clipped.z / clipped.w;
        Maths::Vec2i screenPos = viewport(ndcX, ndcY, m_width, m_height);
        int halfSize = (int)(particle.size * 0.5f + 0.5f);
        if (particle.size < 1.0f) halfSize = 0;
        for (int dy = -halfSize; dy <= halfSize; dy++)
        {
            for (int dx = -halfSize; dx <= halfSize; dx++)
            {
                int px = screenPos.x + dx;
                int py = screenPos.y + dy;
                if (px >= 0 && px < (int)m_width && py >= 0 && py < (int)m_height)
                {
                    if (z > getDepthBuffer()[py * m_width + px])
                    {
                        continue;
                    }
                    uint32_t dst = getFrameBuffer()[py * m_width + px];
                    uint32_t src = particle.color;

                    uint32_t r = std::min(((dst >> 16) & 0xFF) + ((src >> 16) & 0xFF), 255u);
                    uint32_t g = std::min(((dst >> 8) & 0xFF) + ((src >> 8) & 0xFF), 255u);
                    uint32_t b = std::min((dst & 0xFF) + (src & 0xFF), 255u);
                    getFrameBuffer()[py * m_width + px] = 0xFF000000 | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
}

void Renderer::present(SDL_Window* window)
{
    SDL_UpdateWindowSurface(window);
}

void Renderer::clearFrameBuffer(uint32_t color)
{
    std::fill_n(m_framebuffer, m_width * m_height, color);
}

void Renderer::clearDepthBuffer()
{
    std::fill(m_zBuffer.begin(), m_zBuffer.end(), 1e30f);
}

void Renderer::onResize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    initTiles();
    m_zBuffer.resize(width * height + 16, 1e30f);
}

void Renderer::initTiles() {
    int tilesX = (m_width + TILE_SIZE - 1) / TILE_SIZE;
    int tilesY = (m_height + TILE_SIZE - 1) / TILE_SIZE;
    int tileWidth = m_width / tilesX;
    int tileHeight = m_height / tilesY;

    m_tiles.clear();
    for (int ty = 0; ty < tilesY; ty++) {
        for (int tx = 0; tx < tilesX; tx++) {
            m_tiles.push_back({
                tx * tileWidth,
                ty * tileHeight,
                (tx == tilesX - 1) ? (int)m_width : (tx + 1) * tileWidth,
                (ty == tilesY - 1) ? (int)m_height : (ty + 1) * tileHeight
                });
        }
    }

    m_tileBins.resize(m_tiles.size());
}

void Renderer::workerLoop(Worker& w)
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(w.mutex);
        w.cv.wait(lock, [&] { return w.hasWork || w.stop; });
        if (w.stop) break;
        lock.unlock();

        uint32_t tileIdx;
        while ((tileIdx = m_tileCounter.fetch_add(1)) < m_tiles.size())
        {
            auto& tile = m_tiles[tileIdx];
            for (auto& tri : m_tileBins[tileIdx])
            {
                drawTriangle( tri,
                    { m_framebuffer, m_zBuffer.data(), m_width, m_height, {tile.x_min, tile.y_min}, {tile.x_max, tile.y_max} }
                );
            }
        }

        {
            std::lock_guard<std::mutex> lock2(w.mutex);
            w.hasWork = false;
        }
        w.finished.notify_one();
    }
}