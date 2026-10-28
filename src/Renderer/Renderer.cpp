#include "Renderer.h"
#include <algorithm>
#include <thread>

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
    onResize(width, height);  // initTiles() appelée dans onResize

    m_numWorkers = std::max(1, (int)std::thread::hardware_concurrency() - 1);
    m_workers.resize(m_numWorkers);

    for (int i = 0; i < m_numWorkers; i++) {
        m_workers[i] = std::make_unique<Worker>();
        m_workers[i]->thread = std::thread(&Renderer::workerLoop, this, std::ref(*m_workers[i]));
    }
}

void Renderer::renderScene(const Scene& scene)
{

    m_screenTriangles.clear();
    m_opaqueTriangles.clear();
    m_transparentTriangles.clear();

    for (auto & bin : m_tileBins)
    {
        bin.clear();
    }

    auto view = scene.camera.getViewMatrix();
    auto proj = scene.camera.getProjMatrix((float)m_width / m_height);
    auto viewProj = proj * view;


    for (auto& obj : scene.objects)
    {
        if (!obj.mesh) continue;

        auto model = obj.transform.getModelMatrix();
        auto MVP = viewProj * model;

        ShaderUniforms uniforms{ &obj.material, &scene.lights[0], scene.camera.position };

        bool isTransparent = (obj.material.dissolve < 1.0f);
        auto& targetList = isTransparent ? m_transparentTriangles : m_opaqueTriangles;

        drawMesh(*obj.mesh, MVP, model, uniforms, scene.lights[0], isTransparent, targetList);
    }

    if (!m_opaqueTriangles.empty())
    {
        for (auto& tri : m_opaqueTriangles)
        {
            int minX = std::min({(int)tri.a.x, (int)tri.b.x, (int)tri.c.x});
            int maxX = std::max({(int)tri.a.x, (int)tri.b.x, (int)tri.c.x});
            int minY = std::min({(int)tri.a.y, (int)tri.b.y, (int)tri.c.y});
            int maxY = std::max({(int)tri.a.y, (int)tri.b.y, (int)tri.c.y});
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
            int minX = std::min({(int)tri.a.x, (int)tri.b.x, (int)tri.c.x});
            int maxX = std::max({(int)tri.a.x, (int)tri.b.x, (int)tri.c.x});
            int minY = std::min({(int)tri.a.y, (int)tri.b.y, (int)tri.c.y});
            int maxY = std::max({(int)tri.a.y, (int)tri.b.y, (int)tri.c.y});

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

    // Fork
    m_tileCounter = 0;
    for (int i = 0; i < m_numWorkers; i++) {
        std::lock_guard<std::mutex> lock(m_workers[i]->mutex);
        m_workers[i]->hasWork = true;
        m_workers[i]->cv.notify_one();
    }

    // Join
    for (int i = 0; i < m_numWorkers; i++) {
        std::unique_lock<std::mutex> lock(m_workers[i]->mutex);
        m_workers[i]->finished.wait(lock, [&] { return !m_workers[i]->hasWork; });
    }

}

void Renderer::drawMesh(const Mesh& mesh, const Maths::Matrix4x4<>& MVP, const Maths::Matrix4x4<>& model,
    const ShaderUniforms& uniforms, const Light& light,
    bool isTransparent, std::vector<ScreenTriangle>& outTriangles)
{
    size_t vertexCount = mesh.vertices.size();

    m_clipPositions.resize(vertexCount);
    m_worldNormals.resize(vertexCount);
    m_worldPositions.resize(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i)
    {
        m_clipPositions[i] = MVP * Maths::Vec4f(mesh.vertices[i], 1.0f);
        m_worldNormals[i] = model.TransformDirection(mesh.vertexNormals[i]);

        Maths::Vec4f wPos = model * Maths::Vec4f(mesh.vertices[i], 1.0f);
        m_worldPositions[i] = { wPos.x, wPos.y, wPos.z };
    }

    for (auto& f : mesh.faces)
    {
        ClipVertex v0 = { m_clipPositions[f.v.x], mesh.uvs[f.uv.x], m_worldNormals[f.v.x], m_worldPositions[f.v.x] };
        ClipVertex v1 = { m_clipPositions[f.v.z], mesh.uvs[f.uv.z], m_worldNormals[f.v.z], m_worldPositions[f.v.z] };
        ClipVertex v2 = { m_clipPositions[f.v.y], mesh.uvs[f.uv.y], m_worldNormals[f.v.y], m_worldPositions[f.v.y] };

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
            tri.light = &light;

            tri.avgZ = (a.z + b.z + c.z) / 3.0f;
            tri.writeDepth = !isTransparent;

            outTriangles.push_back(tri);
        }
    }
}

void Renderer::drawText(const Font& font, const std::string& text, int x, int y, const Color& color)
{
    ::drawText(font, text, x, y, color, m_framebuffer, m_width, m_height);
}

void Renderer::drawParticle(const ParticleVertex &particle, const Maths::Matrix4x4<> &viewProj)
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
                    uint32_t g = std::min(((dst >>  8) & 0xFF) + ((src >>  8) & 0xFF), 255u);
                    uint32_t b = std::min(( dst        & 0xFF) + ( src        & 0xFF), 255u);
                    getFrameBuffer()[py * m_width + px] = 0xFF000000 | (r << 16) | (g << 8) | b;

                }
            }
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

void Renderer::onResize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    initTiles();
    m_framebuffer.resize(width * height, 0xFF101018);
    m_zBuffer.resize(width * height, 1e30f);
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
                (tx == tilesX - 1) ? (int)m_width  : (tx + 1) * tileWidth,
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
                drawTriangle(
                    tri.a, tri.b, tri.c,
                    tri.uv0, tri.uv1, tri.uv2,
                    tri.invW,
                    tri.n0, tri.n1, tri.n2,
                    tri.wp0, tri.wp1, tri.wp2,
                    m_framebuffer, m_zBuffer,
                    tri.uniforms,
                    *tri.light,
                    {tile.x_min, tile.y_min},
                    {tile.x_max, tile.y_max},
                    tri.writeDepth
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