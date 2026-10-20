#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <cstdint>

#include "Renderer/Renderer.h"

class Engine
{
public:
    Engine() = default;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    virtual ~Engine() = default;

    int  run();
    void stop();

protected:
    virtual void start()  {}
    virtual void update() = 0;


    Renderer m_renderer;

    uint32_t m_clearColor = 0xFF101018; // ARGB dark background

private:
    SDL_Window*   m_window      = nullptr;
    SDL_Renderer* m_sdlRenderer = nullptr;
    SDL_Texture*  m_sdlTexture  = nullptr;
    bool          m_isRunning   = false;
};
