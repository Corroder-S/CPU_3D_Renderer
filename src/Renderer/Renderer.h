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
#include "Engine/Camera.h"
#include "Engine/Transform.h"
#include "Asset/Font.h"
#include "Renderer/Emitter/ParticleManager.h"
#include "Clip.h"
#include "Draw.h"
#include "Pipeline.h"

// ============================================================================
// STRUCTURES INTERNES DU RENDERER
// ============================================================================

/// @brief Thread worker pour le rendu parallèle par tuiles.
struct Worker {
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    std::condition_variable finished;
    bool hasWork = false;
    bool stop = false;
};

/// @brief Région rectangulaire de l'écran assignée à un worker.
struct Tile {
    int x_min, y_min, x_max, y_max;
};

// ============================================================================
// RENDERER PRINCIPAL
// ============================================================================

/// @brief Rasterizer CPU multi-threadé avec pipeline configurable.
///
/// Gère le framebuffer, le z-buffer, le découpage en tuiles et
/// la soumission des meshes opaques et transparents.
class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    /// @brief Initialise le framebuffer, le z-buffer et les workers.
    /// @param width  Largeur en pixels.
    /// @param height Hauteur en pixels.
    void init(uint32_t width, uint32_t height);

    /// @brief Transforme et rasterise un sous-mesh dans la liste de triangles fournie.
    /// @param mesh          Mesh source.
    /// @param submesh       Sous-mesh à dessiner.
    /// @param model         Matrice modèle.
    /// @param uniforms      Uniformes du shader (lumières, caméra…).
    /// @param isTransparent Vrai si le sous-mesh est transparent.
    /// @param outTriangles  Triangles écran produits en sortie.
    void drawSubMesh(const Mesh& mesh, const SubMesh& submesh,
        const Maths::Matrix4x4<>& model, const ShaderUniforms& uniforms,
        bool isTransparent, std::vector<ScreenTriangle>& outTriangles);

    /// @brief Transforme et rasterise tous les sous-meshes d'un mesh.
    /// @param mesh          Mesh source.
    /// @param MVP           Matrice Model-View-Projection.
    /// @param model         Matrice modèle.
    /// @param uniforms      Uniformes du shader.
    /// @param isTransparent Vrai si le mesh est transparent.
    /// @param outTriangles  Triangles écran produits en sortie.
    void drawMesh(const Mesh& mesh, const Maths::Matrix4x4<>& MVP,
        const Maths::Matrix4x4<>& model, const ShaderUniforms& uniforms,
        bool isTransparent, std::vector<ScreenTriangle>& outTriangles);

    /// @brief Prépare la frame : calcule la matrice view-projection et stocke la position caméra.
    /// @param cam         Caméra active.
    /// @param aspectRatio Rapport largeur/hauteur de la fenêtre.
    void beginFrame(const Camera& cam, float aspectRatio);

    /// @brief Enregistre les lumières à utiliser pour la frame courante.
    /// @param lights Tableau de lumières.
    /// @param count  Nombre de lumières.
    void setLights(const Light* lights, uint32_t count);

    /// @brief Soumet un mesh avec ses matériaux pour le rendu de la frame.
    void submitMesh(const Mesh& mesh, const Transform& transform, const std::vector<Material>& materials);

    /// @brief Soumet un mesh sans matériau explicite.
    void submitMesh(const Mesh& mesh, const Transform& transform);

    /// @brief Soumet un mesh avec un matériau unique.
    void submitMesh(const Mesh& mesh, const Transform& transform, const Material& material);

    /// @brief Trie les triangles opaques/transparents puis lance le rendu par tuiles.
    void endFrame();

    /// @brief Rasterise du texte en espace écran.
    /// @param font  Police à utiliser.
    /// @param text  Chaîne à afficher.
    /// @param x     Position X en pixels.
    /// @param y     Position Y en pixels.
    /// @param color Couleur du texte.
    void drawText(const Font& font, const std::string& text, int x, int y, const Color& color);

    /// @brief Projette et dessine une particule.
    /// @param particle  Données de la particule.
    /// @param viewProj  Matrice view-projection.
    void drawParticle(const ParticleVertex& particle, const Maths::Matrix4x4<>& viewProj);

    /// @brief Copie le framebuffer vers la surface SDL et rafraîchit la fenêtre.
    void present(SDL_Window* window);

    /// @brief Remplit le framebuffer avec une couleur unie (format ARGB 32 bits).
    void clearFrameBuffer(uint32_t color);

    /// @brief Réinitialise le z-buffer à 1.0 (plan far).
    void clearDepthBuffer();

    uint32_t* getFrameBuffer() { return m_framebuffer; }
    float* getDepthBuffer() { return m_zBuffer.data(); }
    uint32_t  getWidth()  const { return m_width; }
    uint32_t  getHeight() const { return m_height; }

    void setFrameBuffer(uint32_t* framebuffer) { m_framebuffer = framebuffer; }
    void setParticleManager(ParticleManager* manager) { m_particleManager = manager; }

    SDL_Surface* getSDLSURFACE() { return m_sdlSurface; }
    void setSDLSURFACE(SDL_Surface* surface) { m_sdlSurface = surface; }

    /// @brief Réalloue le framebuffer et le z-buffer après un redimensionnement de fenêtre.
    void onResize(uint32_t width, uint32_t height);

private:
    SDL_Surface* m_sdlSurface = nullptr;
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

    /// @brief Découpe l'écran en tuiles de taille TILE_SIZE.
    void initTiles();

    /// @brief Boucle principale d'un worker : attend du travail, rasterise sa tuile.
    void workerLoop(Worker& w);
};
