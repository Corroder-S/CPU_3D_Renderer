#pragma once

#include <algorithm>
#include <cassert>

#include "ECS/headers/registry.h"


namespace ECS
{
    inline Registry::Registry() : m_records(EntityRecord{nullptr, 0, 0})
    {
        m_emptyArchetype = getOrCreateArchetype({});
    }

    inline EntityID Registry::createEntity()
    {

        EntityID id = m_idGenerator.generateID();
        Chunk* lastChunk = m_emptyArchetype->m_entityColumn.getOrAddChunk(m_chunkPool, m_emptyArchetype->chunkCapacity);
        reinterpret_cast<EntityID*>(lastChunk->m_chunk)[lastChunk->m_count] = id;
        lastChunk->m_count++;
        uint32_t chunkIndex = m_emptyArchetype->m_entityColumn.m_Data.size() - 1;
        uint32_t slotIndex  = lastChunk->m_count - 1;
        m_records.get(id & VALUE_MASK) = { m_emptyArchetype, chunkIndex, slotIndex };
        return id;
    }

    inline void Registry::createEntityWithID(EntityID entity)
    {
        m_idGenerator.forceID(entity);

    }

    inline void Registry::destroyEntity(EntityID entity)
    {
        if (m_idGenerator.isValid(entity))
        {
            m_entitiesToDestroy.push_back(entity);
        }
    }

    inline void Registry::update() {
        for (auto toRemove : m_entitiesToDestroy)
        {
            if (m_idGenerator.isValid(toRemove))
            {
                removeEntity(toRemove);
            }
        }
        m_entitiesToDestroy.clear();
    }

    template<typename T, typename... Args>
    T& Registry::addComponent(EntityID entityID, Args&&... args)
    {
        uint32_t id = ComponentIDGenerator::get<T>();
        ComponentMetaRegistry::reg<T>();
        EntityRecord& record = m_records.get(entityID & VALUE_MASK);
        Archetypes* currentArchetype = record.archetype;
        std::vector<uint32_t> newIDS = currentArchetype->m_componentTypeList;
        if (std::find(newIDS.begin(), newIDS.end(), id) == newIDS.end())
        {
            auto pos = std::lower_bound(newIDS.begin(), newIDS.end(), id);
            newIDS.insert(pos, id);
        }


        Archetypes* newArch = getOrCreateArchetype(newIDS);
        if (currentArchetype != newArch)
        {
            migrateEntity(currentArchetype, newArch, record);
        }
        auto it = std::lower_bound(newArch->m_componentTypeList.begin(), newArch->m_componentTypeList.end(), id);
        size_t col = std::distance(newArch->m_componentTypeList.begin(), it);
        Column& column = newArch->m_componentData[col];
        Chunk* destChunk = column.getOrAddChunk(m_chunkPool, newArch->chunkCapacity);
        std::byte* newData = destChunk->m_chunk + destChunk->m_count * ComponentMetaRegistry::get<T>().stride;
        new(newData) T(std::forward<Args>(args)...);
        destChunk->m_count++;
        return *reinterpret_cast<T*>(newData);
    }

    template<typename T>
    T & Registry::getComponent(EntityID entityID)
    {
        EntityRecord& record = m_records.get(entityID & VALUE_MASK);
        Archetypes* currentArchetype = record.archetype;
        uint32_t id = ComponentIDGenerator::get<T>();
        auto it = std::lower_bound(currentArchetype->m_componentTypeList.begin(), currentArchetype->m_componentTypeList.end(), id);
        if (it == currentArchetype->m_componentTypeList.end() || *it != id)
        {
            assert(false && "getComponent: entity does not have component");
        }
        size_t col = std::distance(currentArchetype->m_componentTypeList.begin(), it);
        Column& column = currentArchetype->m_componentData[col];
        Chunk* chunk = column.m_Data[record.chunkIndex];
        return *reinterpret_cast<T*>(chunk->m_chunk + record.slotIndex * column.stride);
    }

    template<typename T>
    bool Registry::hasComponent(EntityID entityID)
    {
        EntityRecord& record = m_records.get(entityID & VALUE_MASK);
        Archetypes* currentArchetype = record.archetype;

        uint32_t id = ComponentIDGenerator::get<T>();
        auto it = std::lower_bound(currentArchetype->m_componentTypeList.begin(), currentArchetype->m_componentTypeList.end(), id);
        if (it == currentArchetype->m_componentTypeList.end() || *it != id)
        {
            return false;
        }
        return true;
    }

    template<typename T>
    void Registry::removeComponent(EntityID entityID)
    {
        EntityRecord& record = m_records.get(entityID & VALUE_MASK);
        Archetypes* currentArchetype = record.archetype;
        uint32_t id = ComponentIDGenerator::get<T>();
        auto it = std::lower_bound(currentArchetype->m_componentTypeList.begin(), currentArchetype->m_componentTypeList.end(), id);
        if (it == currentArchetype->m_componentTypeList.end() || *it != id)
        {
            assert(false && "removeComponent: entity does not have component");
        }

        std::vector<uint32_t> newIDS = currentArchetype->m_componentTypeList;
        auto eraseIt = std::lower_bound(newIDS.begin(), newIDS.end(), id);
        newIDS.erase(eraseIt);
        Archetypes* newArch = getOrCreateArchetype(newIDS);
        if (currentArchetype != newArch)
        {
            migrateEntity(currentArchetype, newArch, record);
        }

    }


    template<typename... Ts, typename Fn>
    void Registry::forEach(Fn&& fn)
    {
        std::vector<uint32_t> componentIDs = { ComponentIDGenerator::get<Ts>()... };
        for (auto* arch : m_archetypes)
        {
            // Vérifier que l'archetype a tous les composants demandés
            std::vector<Column*> columns;
            bool hasAll = true;
            for (auto id : componentIDs)
            {
                auto it = std::lower_bound(arch->m_componentTypeList.begin(), arch->m_componentTypeList.end(), id);
                if (it == arch->m_componentTypeList.end() || *it != id)
                {
                    hasAll = false;
                    break;
                }
                columns.push_back(&arch->m_componentData[std::distance(arch->m_componentTypeList.begin(), it)]);
            }
            if (!hasAll) continue;
            // Itérer les chunks
            Column& entityCol = arch->m_entityColumn;
            for (size_t chunkIdx = 0; chunkIdx < entityCol.m_Data.size(); ++chunkIdx)
            {
                Chunk* entityChunk = entityCol.m_Data[chunkIdx];
                for (size_t slotIdx = 0; slotIdx < entityChunk->m_count; ++slotIdx)
                {
                    EntityID entityID = reinterpret_cast<EntityID*>(entityChunk->m_chunk)[slotIdx];
                    [&]<size_t... Is>(std::index_sequence<Is...>)
                    {
                        fn(entityID,
                           *reinterpret_cast<Ts*>(
                               columns[Is]->m_Data[chunkIdx]->m_chunk + slotIdx * columns[Is]->stride
                           )...
                        );
                    }(std::index_sequence_for<Ts...>{});
                }
            }
        }
    }


    Archetypes* Registry::getOrCreateArchetype(const  std::vector<uint32_t>& componentIDs)
    {
        for (auto *arch : m_archetypes)
        {
            if (arch->m_componentTypeList == componentIDs)
            {
                return arch;
            }

        }
        Archetypes* newArch = new Archetypes();
        newArch->m_componentTypeList = componentIDs;
        size_t strideMax = 0;
        for (auto& id : componentIDs)
        {
            strideMax = std::max(strideMax, ComponentMetaRegistry::get(id).stride);
        }
        if (strideMax == 0)
        {
            newArch->chunkCapacity = CHUNK_SIZE / sizeof(EntityID);
        }
        else
        {
            newArch->chunkCapacity = CHUNK_SIZE / strideMax;
        }
        for (auto& id : componentIDs)
        {
            size_t stride = ComponentMetaRegistry::get(id).stride;
            newArch->m_componentData.emplace_back();
            newArch->m_componentData.back().stride = stride;
        }
        newArch->m_entityColumn.stride = sizeof(EntityID);
        m_archetypes.push_back(newArch);
        return newArch;
    }

    inline void Registry::migrateEntity(Archetypes *src, Archetypes *dest, EntityRecord &record)
    {

        Column& entityCol = src->m_entityColumn;
        Chunk* lastEntityChunk = entityCol.m_Data.back();
        uint32_t lastSlot = lastEntityChunk->m_count - 1;
        EntityID lastEntity = reinterpret_cast<EntityID*>(lastEntityChunk->m_chunk)[lastSlot];
        EntityID migratedEntity = reinterpret_cast<EntityID*>(src->m_entityColumn.m_Data[record.chunkIndex]->m_chunk)[record.slotIndex];
        uint32_t originalChunkIndex = record.chunkIndex;
        uint32_t originalSlotIndex  = record.slotIndex;

        for (size_t index = 0; index < src->m_componentData.size(); index++)
        {
            Column& srcCol =  src->m_componentData[index];
            std::byte* srcPtr = srcCol.m_Data[record.chunkIndex]->m_chunk + record.slotIndex * srcCol.stride;


            //move comp
            uint32_t compID = src->m_componentTypeList[index];

            auto it = std::lower_bound(dest->m_componentTypeList.begin(), dest->m_componentTypeList.end(), compID);
            const ComponentMeta& meta = ComponentMetaRegistry::get(compID);
            if (it == dest->m_componentTypeList.end() || *it != compID)
            {
                meta.fnDestruct(srcPtr); // pas dans dest → juste détruire
            }
            else
            {
                size_t destColIndex = std::distance(dest->m_componentTypeList.begin(), it);
                Chunk* destChunk = dest->m_componentData[destColIndex].getOrAddChunk(m_chunkPool, dest->chunkCapacity);
                std::byte* destPtr = destChunk->m_chunk + destChunk->m_count * srcCol.stride;
                meta.fnMoveConstruct(destPtr, srcPtr);
                meta.fnDestruct(srcPtr);
                destChunk->m_count++;
            }
            //swap Comp if needed
            Chunk* lastChunk = srcCol.m_Data.back();
            if (lastEntity != migratedEntity)
            {
                std::byte* lastPtr = lastChunk->m_chunk + lastSlot* srcCol.stride;
                meta.fnMoveConstruct(srcPtr, lastPtr);
                meta.fnDestruct(lastPtr);


            }
            lastChunk->m_count--;
            if (lastChunk->m_count == 0)
            {
                m_chunkPool.freeChunk(lastChunk->m_chunk);
                delete lastChunk;
                srcCol.m_Data.pop_back();
            }
        }

        Chunk* destEntityChunk = dest->m_entityColumn.getOrAddChunk(m_chunkPool, dest->chunkCapacity);
        reinterpret_cast<EntityID*>(destEntityChunk->m_chunk)[destEntityChunk->m_count] = migratedEntity;
        destEntityChunk->m_count++;

        if ( lastEntity != migratedEntity)
        {
            reinterpret_cast<EntityID*>(entityCol.m_Data[originalChunkIndex]->m_chunk)[originalSlotIndex] = lastEntity;
            m_records.get(lastEntity & VALUE_MASK) = { src, originalChunkIndex, originalSlotIndex };
        }
        lastEntityChunk->m_count--;
        if (lastEntityChunk->m_count == 0)
        {
            m_chunkPool.freeChunk(lastEntityChunk->m_chunk);
            delete lastEntityChunk;
            entityCol.m_Data.pop_back();
        }
        record.archetype = dest;
        record.chunkIndex = dest->m_entityColumn.m_Data.size() - 1;
        record.slotIndex  = destEntityChunk->m_count - 1;
    }

    inline void Registry::removeEntity(EntityID toRemove)
    {
        EntityRecord& record = m_records.get(toRemove & VALUE_MASK);
        migrateEntity(record.archetype, m_emptyArchetype, record);
        m_idGenerator.destroyEntity(toRemove);
    }
}
