#pragma once
#include <algorithm>
#include <cstdint>

#include "entity_component.h"

// ============================================================================
// REGISTRE DE MÉTADONNÉES DE COMPOSANTS
// ============================================================================

/// @brief Nombre maximal de types de composants supportés simultanément.
const uint32_t MAX_COMPONENTS = 32;

/// @brief Métadonnées d'un type de composant : taille et opérations de cycle de vie.
struct ComponentMeta {
    size_t stride;
    void (*fnConstruct)(void* dest);
    void (*fnDestruct)(void* ptr);
    void (*fnMoveConstruct)(void* dest, void* src);
};

/// @brief Registre statique associant chaque typeID à ses ComponentMeta.
///
/// Permet d'effectuer des opérations génériques (construction, destruction,
/// déplacement) sur des composants dont le type n'est connu qu'à l'exécution.
class ComponentMetaRegistry
{
public:

    /// @brief Enregistre les métadonnées du type T si elles ne le sont pas encore.
    /// @tparam T Type de composant à enregistrer.
    template <typename T>
    static void ensureReg()
    {
        uint32_t id = ECS::ComponentIDGenerator::get<T>();
        if (id >= componentMeta.size())
            componentMeta.resize(id + 1);
        componentMeta[id].stride = sizeof(T);
        componentMeta[id].fnConstruct = [](void* dest) { new (dest) T(); };
        componentMeta[id].fnDestruct = [](void* ptr) { static_cast<T*>(ptr)->~T(); };
        componentMeta[id].fnMoveConstruct = [](void* dest, void* src) { new (dest) T(std::move(*static_cast<T*>(src))); };
    }

    /// @brief Accès aux métadonnées d'un type connu à la compilation.
    /// @tparam T Type de composant.
    /// @return Référence vers les ComponentMeta correspondantes.
    template <typename T>
    static const ComponentMeta& get()
    {
        uint32_t id = ECS::ComponentIDGenerator::get<T>();
        return componentMeta[id];
    }

    /// @brief Accès aux métadonnées d'un type connu uniquement à l'exécution.
    /// @param id Identifiant numérique du type de composant.
    /// @return Référence vers les ComponentMeta correspondantes.
    static const ComponentMeta& get(uint32_t id)
    {
        return componentMeta[id];
    }

private:
    inline static std::vector<ComponentMeta> componentMeta;

};
