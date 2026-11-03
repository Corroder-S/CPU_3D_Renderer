#pragma once

// Internals
#include <ECS/headers/entity_component.h>

// STL
#include <memory>

#include "Archetypes.h"
#include "ChunkPool.h"
#include "paged_allocator.h"

namespace ECS
{

    class Registry {
    public:
        Registry();
        ~Registry() = default;

        EntityID createEntity();


        void createEntityWithID(EntityID entity);

        void destroyEntity(EntityID entity);

         void update();

        template<typename T, typename... Args>
        T& addComponent(EntityID entityID, Args&&... args);


        Archetypes *getOrCreateArchetype(const std::vector<uint32_t> &componentIDs);

        void migrateEntity(Archetypes* src, Archetypes* dest, EntityRecord& record);

        template <typename T>
        T& getComponent(EntityID entityID);


        template <typename T>
        bool hasComponent(EntityID entityID);

        template <typename T>
        void removeComponent(EntityID entityID);

        template<typename... Ts, typename Fn>
        void forEach(Fn&& fn);

    private:
        EntityIDGenerator m_idGenerator;
        PagedAllocator<EntityRecord> m_records;
        std::vector<Archetypes*> m_archetypes;
        Archetypes* m_emptyArchetype = nullptr;
        ChunkPool m_chunkPool;

        std::vector<EntityID> m_entitiesToDestroy;

        void removeEntity(EntityID toRemove);
    };
}

#include <ECS/detail/registry.inl>
