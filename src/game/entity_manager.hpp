#pragma once

#include <algorithm>
#include <unordered_map>

#include "ecs.hpp"
#include "../common_util/misc.hpp"

namespace pge
{

    class EntityManager
    {
    public:
        using EntityTable = std::unordered_map<std::string, Entity, ENABLE_TRANSPARENT_HASH>;
        void start();

        void update(double delta_time);

        Entity* find(std::string_view name);

        template<IsComponent ...C>
        Entity* create(std::string &&name)
        {
            auto [iter, inserted] = m_entities.emplace(std::forward<std::string>(name), Entity{});

            if (!inserted)
            {
                return nullptr;
            }

            Entity &entity = iter->second;

            entity.m_name = iter->first;

            ((entity.register_component<C>()), ...);

            return &entity;
        }

        EntityTable& get_entities()
        {
            return m_entities;
        }

    private:
        EntityTable m_entities;
    };
}