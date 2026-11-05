//
// Created by leoam on 03/11/2026.
//

#include "RendererSystem.h"

#include "../Component.h"

void RendererSystem::update(float deltaTime)
{
    Camera* activeCamera = nullptr;
    m_registry.forEach<CameraComponent, ActiveCameraTag>([&](ECS::EntityID, CameraComponent& cameraComponent, ActiveCameraTag&)
        {
            activeCamera = &cameraComponent.camera;
        });
    if (!activeCamera)
    {
        return;
    }

    std::vector<Light> lights;
    m_registry.forEach<LightComponent>([&](ECS::EntityID, LightComponent& lightComponent)
        {
            lights.push_back(lightComponent.light);
        });

    if (lights.empty())
    {
        return;
    }

    float aspect = (float)m_renderer.getWidth() / (float)m_renderer.getHeight();
    m_renderer.beginFrame(*activeCamera, aspect);
    m_renderer.setLights(lights.data(), (uint32_t)lights.size());

    m_registry.forEach<Transform, MeshComponent, MaterialComponent>([&](ECS::EntityID, Transform& transform, MeshComponent& meshComponent, MaterialComponent& materialComponent)
        {
            if (!meshComponent.mesh) return;
            m_renderer.submitMesh(*meshComponent.mesh, transform, materialComponent.materials);
        });

    m_renderer.endFrame();
}