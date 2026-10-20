#include "Time.h"

Time Time::s_instance;

void Time::update()
{
    auto now = std::chrono::high_resolution_clock::now();
    s_instance.m_deltaTime = std::chrono::duration<float>(now - s_instance.m_lastTime).count();
    s_instance.m_lastTime  = now;
    s_instance.m_fps       = (s_instance.m_deltaTime > 0.0f) ? 1.0f / s_instance.m_deltaTime : 0.0f;
}
