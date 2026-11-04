#pragma once
#include <algorithm>
#include <cstdint>

#include "entity_component.h"


const uint32_t MAX_COMPONENTS = 32;


struct ComponentMeta {
    size_t stride;
    void (*fnConstruct)(void* dest);
    void (*fnDestruct)(void* ptr);
    void (*fnMoveConstruct)(void* dest, void* src);
};

class ComponentMetaRegistry
{
public:

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
    template <typename T> //compile time
    static const ComponentMeta& get()
    {
        uint32_t id = ECS::ComponentIDGenerator::get<T>();
        return componentMeta[id];
    }

    static const ComponentMeta& get(uint32_t id) //runtime
    {
        return componentMeta[id];
    }

private:
    inline static std::vector<ComponentMeta> componentMeta;

};