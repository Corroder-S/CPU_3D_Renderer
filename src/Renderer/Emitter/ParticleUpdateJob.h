#pragma once
#include <vector>

#include "MasterParticleVertexBuffer.h"
#include "ParticleSystem.h"

// ============================================================================
// TÂCHE DE MISE À JOUR DE PARTICULES
// ============================================================================

/// @brief Unité de travail exécutée par le WorkerPool pour un pool de particules.
/// Met à jour les systèmes de son pool et pousse les vertices billboardés
/// dans le MasterParticleVertexBuffer.
class ParticleUpdateJob
{
public:

    std::vector<ParticleSystem*>* particleSystems; ///< Systèmes à mettre à jour.
    MasterParticleVertexBuffer* masterBuffer;       ///< Buffer de destination des vertices.
    uint32_t poolIndex;                             ///< Index du pool traité par cette tâche.

    float deltaTime;
    Maths::Vec3<float> cameraDirection; ///< Direction de la caméra (pour l'orientation des billboards).
    Maths::Vec3<float> cameraPosition;  ///< Position de la caméra (pour le tri par profondeur).

    /// @brief Exécute la mise à jour et remplit le buffer.
    virtual void execute();
};
