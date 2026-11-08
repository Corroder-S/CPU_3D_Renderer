#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <cstdint>

#include "Renderer/Renderer.h"

// ============================================================================
// ENGINE
// ============================================================================

/// @brief Classe de base du moteur : gère la boucle principale SDL et le cycle de vie de l'application.
///
/// À dériver pour implémenter la logique applicative via start() et update().
class Engine
{
public:
    Engine() = default;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    virtual ~Engine() = default;

    /// @brief Lance la boucle principale (initialisation SDL, boucle d'événements, rendu).
    /// @return Code de retour : 0 si succès, non-zéro si erreur.
    int  run();

    /// @brief Demande l'arrêt de la boucle principale.
    void stop();

protected:
    /// @brief Appelée une fois avant la première frame — surcharger pour initialiser la scène.
    virtual void start()  {}

    /// @brief Appelée chaque frame — surcharger pour implémenter la logique de mise à jour.
    virtual void update() = 0;

    Renderer m_renderer;

    uint32_t m_clearColor = 0xFF101018; // Couleur de fond ARGB (noir bleuté)

private:
    SDL_Window*   m_window      = nullptr;
    SDL_Renderer* m_sdlRenderer = nullptr;
    SDL_Texture*  m_sdlTexture  = nullptr;
    bool          m_isRunning   = false;
};
