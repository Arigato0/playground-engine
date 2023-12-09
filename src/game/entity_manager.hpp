#pragma once

#include <algorithm>
#include <unordered_map>

#include "ecs.hpp"

namespace pge
{
    class EntityManager
    {
    public:
        using EntityTable = std::unordered_map<std::string_view, std::unique_ptr<Entity>>;
        void start();

        void update(double delta_time);

        Entity* find(std::string_view name);

        template<IsComponent ...C>
        Entity* create(std::string_view name)
        {
            auto ptr = std::make_unique<Entity>(std::string{ name });

            auto [iter, inserted] = m_entities.emplace(ptr->name(), std::move(ptr));

            if (!inserted)
            {
                return nullptr;
            }

            Entity *entity = iter->second.get();

            ((entity->register_component<C>()), ...);

            return entity;
        }

        const EntityTable& get_entities() const
        {
            return m_entities;
        }

    private:
        EntityTable m_entities;

    };
}