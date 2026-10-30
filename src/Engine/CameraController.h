#pragma once

#include "Scene/Camera.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <numbers>

class CameraController
{
private:
    float m_moveSpeed = 10.0f;
    float m_sprintMultiplier = 2.5f;
    float m_sensitivity = 0.15f;

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    bool  m_initialized = false;

public:
    CameraController(float speed = 10.0f, float sensitivity = 0.15f) : m_moveSpeed(speed), m_sensitivity(sensitivity) {}

    void update(Camera& camera, float dt)
    {
        Maths::Vec3f forward = camera.target - camera.position;
        float lenSq = Maths::Dot(forward, forward);

        if (!m_initialized && lenSq > 0.0001f)
        {
            forward = Maths::Normalize(forward);
            m_pitch = std::asin(forward.y) * (180.0f / 3.14159265f);
            m_yaw = std::atan2(forward.z, forward.x) * (180.0f / 3.14159265f);
            m_initialized = true;
        }

        float deltaX = 0.0f;
        float deltaY = 0.0f;
        Uint32 mouseState = SDL_GetRelativeMouseState(&deltaX, &deltaY);

        if (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT))
        {
            m_yaw += deltaX * m_sensitivity;
            m_pitch -= deltaY * m_sensitivity;

            if (m_pitch > 89.0f)  m_pitch = 89.0f;
            if (m_pitch < -89.0f) m_pitch = -89.0f;

            float yawRad = m_yaw * (std::numbers::pi / 180.0f);
            float pitchRad = m_pitch * (std::numbers::pi / 180.0f);

            forward.x = std::cos(yawRad) * std::cos(pitchRad);
            forward.y = std::sin(pitchRad);
            forward.z = std::sin(yawRad) * std::cos(pitchRad);
            forward = Maths::Normalize(forward);

            camera.target = camera.position + forward;
        }
        else if (lenSq > 0.0001f)
        {
            forward = Maths::Normalize(forward);
        }
        else
        {
            forward = { 0.0f, 0.0f, -1.0f };
        }

        const bool* keyState = SDL_GetKeyboardState(NULL);

        Maths::Vec3f right = Maths::Cross(forward, Maths::Vec3f{ 0.0f, 1.0f, 0.0f });
        if (Maths::Dot(right, right) > 0.0f)
        {
            right = Maths::Normalize(right);
        }
        else
        {
            right = { 1.0f, 0.0f, 0.0f };
        }

        Maths::Vec3f up{ 0.0f, 1.0f, 0.0f };

        float speed = m_moveSpeed * dt;
        if (keyState[SDL_SCANCODE_LSHIFT])
        {
            speed *= m_sprintMultiplier;
        }

        Maths::Vec3f move{ 0.0f, 0.0f, 0.0f };

        if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_Z]) move += forward * speed;
        if (keyState[SDL_SCANCODE_S])                          move -= forward * speed;
        if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_Q]) move -= right * speed;
        if (keyState[SDL_SCANCODE_D])                          move += right * speed;
        if (keyState[SDL_SCANCODE_SPACE])                      move += up * speed;
        if (keyState[SDL_SCANCODE_LCTRL])                      move -= up * speed;

        camera.position += move;
        camera.target += move;
    }

    void setMoveSpeed(float speed) { m_moveSpeed = speed; }
    void setSensitivity(float sensitivity) { m_sensitivity = sensitivity; }
};