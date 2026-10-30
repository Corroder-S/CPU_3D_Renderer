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

    m_sdlRenderer = SDL_CreateRenderer(m_window, nullptr);
    m_sdlTexture  = SDL_CreateTexture(
        m_sdlRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH, RENDER_HEIGHT
    );
    SDL_SetTextureScaleMode(m_sdlTexture, SDL_SCALEMODE_NEAREST);

    m_renderer.init(RENDER_WIDTH, RENDER_HEIGHT);

    start();

    m_isRunning = true;
    SDL_Event event;

    float fpsAccum = 0.0f;
    int   fpsCount = 0;
    char  title[160];

    while (m_isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                m_isRunning = false;
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT)
                SDL_SetWindowRelativeMouseMode(m_window, true);
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_RIGHT)
                SDL_SetWindowRelativeMouseMode(m_window, false);
        }

        Time::update();

        m_renderer.clearFrameBuffer(m_clearColor);
        m_renderer.clearDepthBuffer();

        update();

        m_renderer.present(m_sdlTexture, m_sdlRenderer);

        fpsAccum += Time::deltaTime();
        fpsCount++;
        if (fpsAccum >= 1.0f)
        {
            float fps = fpsCount / fpsAccum;
            std::snprintf(title, sizeof(title), "%s  |  %.1f FPS  (%.2f ms/frame)",
                WINDOW_TITLE, fps, 1000.0f / fps);
            SDL_SetWindowTitle(m_window, title);
            fpsAccum = 0.0f;
            fpsCount = 0;
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
