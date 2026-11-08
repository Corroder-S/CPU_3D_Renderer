#pragma once

// Internals
#include <ECS/headers/entity_component.h>
#include <ECS/headers/json_archive.hpp>

// STL
#include <functional>
#include <string>
#include <vector>

// Forward declarations
namespace SER
{
    class IArchive;
    class JsonWriteArchive;
    class JsonReadArchive;
}

// ============================================================================
// REGISTRE DE COMPOSANTS — SÉRIALISATION ET RÉFLEXION
// ============================================================================

namespace ECS
{
    class Registry;
    using EntityID = std::uint32_t;
    using SER_IArchive = ::gcep::SER::IArchive;
    using SER_JsonWriteArchive = ::gcep::SER::JsonWriteArchive;
    using SER_JsonReadArchive = ::gcep::SER::JsonReadArchive;

    /// @brief Registre singleton recensant tous les types de composants connus.
    ///
    /// Chaque type enregistré expose des fonctions de sérialisation/désérialisation
    /// JSON et de création de pool, permettant une gestion générique des composants.
    class ComponentRegistry
    {
    public:
        /// @brief Entrée du registre pour un type de composant.
        struct Entry
        {
            uint32_t    typeID;
            std::string name;
            /// @brief Garantit l'existence du pool de ce composant dans le Registry.
            std::function<void(Registry&)>                                                   ensurePool;
            /// @brief Désérialise le composant depuis une archive binaire générique.
            std::function<void(Registry&, EntityID, SER_IArchive&)>                          deserialize;
            /// @brief Sérialise tous les composants de ce type en JSON.
            std::function<void(Registry&, SER_JsonWriteArchive&)>                            serializeJson;
            /// @brief Sérialise le composant d'une entité donnée en JSON.
            std::function<void(Registry&, EntityID, SER_JsonWriteArchive&)>                  serializeEntityJson;
            /// @brief Désérialise le composant d'une entité depuis des données JSON parsées.
            std::function<void(Registry&, EntityID, const SER_JsonReadArchive::EntityData&)> deserializeJson;
        };

        /// @brief Retourne l'instance unique du registre.
        static ComponentRegistry& instance();

        /// @brief Enregistre le type T et génère ses fonctions de sérialisation.
        /// @tparam T Type de composant à enregistrer.
        /// @return Vrai si l'enregistrement a réussi.
        template<typename T>
        bool reg();

        /// @brief Retourne la liste de toutes les entrées enregistrées.
        const std::vector<Entry>& entries() const;

    private:
        ComponentRegistry() = default;
        ComponentRegistry(const ComponentRegistry&)            = delete;
        ComponentRegistry& operator=(const ComponentRegistry&) = delete;
        ComponentRegistry(ComponentRegistry&&)                 = delete;
        ComponentRegistry& operator=(ComponentRegistry&&)      = delete;

        std::vector<Entry> m_entries;
    };
} // namespace gcep::ECS

#include <ECS/detail/component_registry.inl>
