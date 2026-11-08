#pragma once

// Internals
#include <ECS/headers/entity_component.h>

// STL
#include <memory>

#include "Archetypes.h"
#include "ChunkPool.h"
#include "paged_allocator.h"

// ============================================================================
// REGISTRY ECS
// ============================================================================

namespace ECS
{

    /// @brief Conteneur central de l'ECS : gère entités, composants et archétypes.
    ///
    /// Les entités sont organisées par archétype (ensemble unique de types de
    /// composants). Les opérations d'ajout/suppression de composants migrent
    /// l'entité vers l'archétype correspondant.
    class Registry {
    public:
        Registry();
        ~Registry() = default;

        /// @brief Crée une nouvelle entité vide et retourne son identifiant.
        EntityID createEntity();

        /// @brief Crée une entité en forçant un ID précis (utile à la désérialisation).
        /// @param entity Identifiant désiré.
        void createEntityWithID(EntityID entity);

        /// @brief Marque une entité pour destruction en fin de frame.
        /// @param entity Entité à détruire.
        void destroyEntity(EntityID entity);

        /// @brief Détruit les entités marquées durant la frame.
        void update();

        /// @brief Ajoute un composant de type T à l'entité et migre vers le nouvel archétype.
        /// @tparam T    Type du composant.
        /// @tparam Args Types des arguments de construction.
        /// @param entityID Entité cible.
        /// @param args     Arguments transmis au constructeur de T.
        /// @return Référence vers le composant créé.
        template<typename T, typename... Args>
        T& addComponent(EntityID entityID, Args&&... args);

        /// @brief Ajoute plusieurs composants en une seule migration d'archétype.
        /// @tparam Ts    Types des composants.
        /// @param entityID   Entité cible.
        /// @param components Valeurs des composants à copier/déplacer.
        template<class ... Ts>
        void addComponents(EntityID entityID, Ts &&... components);

        /// @brief Retourne l'archétype correspondant à la liste de types donnée, ou le crée.
        /// @param componentIDs Liste triée des IDs de types de composants.
        /// @return Pointeur vers l'archétype (non nul).
        Archetypes *getOrCreateArchetype(const std::vector<uint32_t> &componentIDs);

        /// @brief Déplace une entité de l'archétype source vers l'archétype destination.
        /// @param src    Archétype source.
        /// @param dest   Archétype destination.
        /// @param record Localisation actuelle de l'entité (mis à jour en place).
        void migrateEntity(Archetypes* src, Archetypes* dest, EntityRecord& record);

        /// @brief Accède au composant de type T d'une entité.
        /// @tparam T Type du composant.
        /// @param entityID Entité cible.
        /// @return Référence vers le composant.
        template <typename T>
        T& getComponent(EntityID entityID);

        /// @brief Vérifie si une entité possède le composant de type T.
        /// @tparam T Type du composant.
        /// @param entityID Entité à tester.
        /// @return Vrai si le composant est présent.
        template <typename T>
        bool hasComponent(EntityID entityID);

        /// @brief Retire le composant de type T et migre l'entité vers le nouvel archétype.
        /// @tparam T Type du composant à supprimer.
        /// @param entityID Entité cible.
        template <typename T>
        void removeComponent(EntityID entityID);

        /// @brief Itère sur toutes les entités possédant exactement les composants Ts.
        /// @tparam Ts Types de composants requis.
        /// @tparam Fn Type du callable (void(Ts&...)).
        /// @param fn Fonction appelée pour chaque entité correspondante.
        template<typename... Ts, typename Fn>
        void forEach(Fn&& fn);

    private:
        EntityIDGenerator m_idGenerator;
        PagedAllocator<EntityRecord> m_records;
        std::vector<Archetypes*> m_archetypes;
        Archetypes* m_emptyArchetype = nullptr;
        ChunkPool m_chunkPool;

        std::vector<EntityID> m_entitiesToDestroy;

        /// @brief Effectue la destruction immédiate d'une entité.
        void removeEntity(EntityID toRemove);

        /// @brief Place un composant dans le chunk courant de l'archétype.
        template<class T>
        T &placeComponent(Archetypes *arch, T &&component);
    };
}

#include <ECS/detail/registry.inl>
