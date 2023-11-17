#pragma once

#include <unordered_map>

#include "../game/ecs.hpp"

namespace pge
{
    class EntityManager
    {
    public:
        void start();

        void update(double delta_time);

        void register_entity(std::string_view name, Entity *entity);

        Entity* find(std::string_view name);

    private:
        std::unordered_map<std::string_view, Entity*> m_entities;
    };
}