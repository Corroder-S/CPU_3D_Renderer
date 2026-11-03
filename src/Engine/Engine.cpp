#include "Engine.h"
#include "Config.h"
#include "Time.h"

#include <iostream>
#include <cstdio>
#include <chrono>

using EClock = std::chrono::high_resolution_clock;
static double eToMs(EClock::time_point a, EClock::time_point b)
{
    return std::chrono::duration<double, std::milli>(b - a).count();
}

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
    double tClearAccum = 0, tUpdateAccum = 0, tSdlAccum = 0;

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
        }

        Time::update();

        auto tc0 = EClock::now();
        m_renderer.clearFrameBuffer(m_clearColor);
        m_renderer.clearDepthBuffer();
        auto tc1 = EClock::now();

        update();
        auto tc2 = EClock::now();

        SDL_UpdateWindowSurface(m_window);
        auto tc3 = EClock::now();

        tClearAccum  += eToMs(tc0, tc1);
        tUpdateAccum += eToMs(tc1, tc2);
        tSdlAccum    += eToMs(tc2, tc3);

        fpsAccum += Time::deltaTime();
        fpsCount++;
        if (fpsAccum >= 1.0f)
        {
            float fps   = fpsCount / fpsAccum;
            double n    = fpsCount;
            double updateAvg = tUpdateAccum / n;
            double sceneMs   = m_renderer.getTimeSceneMs();
            std::snprintf(title, sizeof(title),
                "%s  |  %.0f FPS  |  clear:%.2f  update:%.2f (scene:%.2f [clrBins:%.2f mesh:%.2f bin:%.2f raster:%.2f] particles:%.2f)  sdl:%.2f  ms per frame:%.2f",
                WINDOW_TITLE, fps,
                tClearAccum / n,
                updateAvg,
                sceneMs,
                m_renderer.getTimeClearBinsMs(),
                m_renderer.getTimeMeshMs(),
                m_renderer.getTimeBinMs(),
                m_renderer.getTimeRasterMs(),
                m_timeParticlesMs,
                tSdlAccum / n,
                std::chrono::duration<float, std::milli>(tc3 - tc0).count());
            SDL_SetWindowTitle(m_window, title);
            fpsAccum = fpsCount = 0;
            tClearAccum = tUpdateAccum = tSdlAccum = 0;
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
