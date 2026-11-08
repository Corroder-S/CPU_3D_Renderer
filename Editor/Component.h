#pragma once
#include "Asset/Light.h"
#include "Asset/Material.h"
#include "Asset/Mesh.h"
#include "Maths/Vec3.h"
#include "Renderer/Emitter/EmitterConfig.h"
#include "Renderer/Emitter/ParticleSystem.h"
#include "Engine/Transform.h"
#include "UI/UICanvas.h"
#include "Engine/Camera.h"

// ============================================================================
// COMPOSANTS ECS DE L'ÉDITEUR
// ============================================================================

/// @brief Référence vers le mesh à afficher pour cette entité.
struct MeshComponent
{
    Mesh* mesh = nullptr;
};

/// @brief Liste des matériaux associés aux sous-meshes de l'entité.
struct MaterialComponent
{
    std::vector<Material> materials;
};

/// @brief Source lumineuse attachée à une entité.
struct LightComponent
{
    Light light;
};

/// @brief Référence vers le système de particules piloté par cette entité.
struct ParticleSystemComponent
{
    ParticleSystem* system = nullptr;
};

/// @brief Caméra de rendu attachée à une entité.
struct CameraComponent
{
    Camera camera;
};

/// @brief Tag indiquant que la caméra de cette entité est la caméra active.
struct ActiveCameraTag {};

/// @brief Transform (position, rotation, échelle) d'une entité dans le monde.
struct TransformComponent
{
    Transform transform;
};

/// @brief Canvas UI associé à cette entité.
struct CanvaComponent
{
    Canva canva;
};

/// @brief Composant texte à afficher via le UICanvas.
struct TextComponent
{
    Text text;
};

/// @brief Composant image à afficher via le UICanvas.
struct ImageComponent
{
    Image image;
};
