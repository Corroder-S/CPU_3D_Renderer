#pragma once

#include <chrono>

// ============================================================================
// TIME
// ============================================================================

/// @brief Singleton de gestion du temps : calcule le deltaTime et les FPS entre chaque frame.
class Time
{
public:
    Time(const Time&) = delete;
    Time& operator=(const Time&) = delete;

    /// @brief Met à jour le deltaTime et les FPS — à appeler une fois par frame.
    static void update();

    /// @return Durée de la dernière frame en secondes.
    static float deltaTime() { return s_instance.m_deltaTime; }

    /// @return Nombre de frames par seconde calculé sur la dernière frame.
    static float fps()       { return s_instance.m_fps; }

private:
    Time() = default;

    float m_deltaTime = 0.0f;
    float m_fps       = 0.0f;

    std::chrono::high_resolution_clock::time_point m_lastTime
        = std::chrono::high_resolution_clock::now();

    static Time s_instance;
};
