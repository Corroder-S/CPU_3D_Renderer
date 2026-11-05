#include "Engine.h"
#include "Config.h"
#include "Time.h"

#include <iostream>
#include <cstdio>

int Engine::run()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return -1;
    }

    m_window = SDL_CreateWindow(WINDOW_TITLE, RENDER_WIDTH, RENDER_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    auto* surface = SDL_GetWindowSurface(m_window);
    m_renderer.init(RENDER_WIDTH, RENDER_HEIGHT);
    m_renderer.setFrameBuffer(reinterpret_cast<uint32_t*>(surface->pixels));

    m_sdlTexture  = SDL_CreateTexture(
        m_sdlRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH, RENDER_HEIGHT
    );
    SDL_SetTextureScaleMode(m_sdlTexture, SDL_SCALEMODE_NEAREST);


    start();

    m_isRunning = true;
    SDL_Event event;

    float fpsAccum = 0.0f;
    int   fpsCount = 0;
    char  title[256];

    while (m_isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                m_isRunning = false;
            else if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                auto* surface = SDL_GetWindowSurface(m_window);
                m_renderer.setFrameBuffer(reinterpret_cast<uint32_t*>(surface->pixels));
                m_renderer.onResize(event.window.data1, event.window.data2);
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT)
                SDL_SetWindowRelativeMouseMode(m_window, true);
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_RIGHT)
                SDL_SetWindowRelativeMouseMode(m_window, false);
        }

        Time::update();

        m_renderer.clearFrameBuffer(m_clearColor);
        m_renderer.clearDepthBuffer();
        update();
        SDL_UpdateWindowSurface(m_window);

        fpsAccum += Time::deltaTime();
        fpsCount++;
        if (fpsAccum >= 1.0f)
        {
            float fps = fpsCount / fpsAccum;
            std::snprintf(title, sizeof(title), "%s  |  %.0f FPS", WINDOW_TITLE, fps);
            SDL_SetWindowTitle(m_window, title);
            fpsAccum = fpsCount = 0;
        }
    }

    SDL_DestroyTexture(m_sdlTexture);
    SDL_DestroyRenderer(m_sdlRenderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
    return 0;
}

void Engine::stop()
{
    m_isRunning = false;
}
