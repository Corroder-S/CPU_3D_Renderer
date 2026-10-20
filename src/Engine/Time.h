#pragma once

#include <chrono>

class Time
{
public:
    Time(const Time&) = delete;
    Time& operator=(const Time&) = delete;

    static void update();

    static float deltaTime() { return s_instance.m_deltaTime; }
    static float fps()       { return s_instance.m_fps; }

private:
    Time() = default;

    float m_deltaTime = 0.0f;
    float m_fps       = 0.0f;

    std::chrono::high_resolution_clock::time_point m_lastTime
        = std::chrono::high_resolution_clock::now();

    static Time s_instance;
};
