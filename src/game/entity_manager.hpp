#pragma once

#include <unordered_map>

#include "ecs.hpp"

namespace pge
{
    template<class T>
    class TypedArena
    {
    public:
        TypedArena(size_t size)
        {
            m_data = (uint8_t*)malloc(sizeof(T) * size);
        }
    private:
        uint8_t *m_data;
    };
    class EntityManager
    {
    public:
        void start();

        void update(double delta_time);

        IEntity* find(std::string_view name);

        template<IsEntity T, IsComponent ...C>
        IEntity* create(std::string_view name)
        {
            auto [iter, inserted] = m_entities.emplace(name, std::make_unique<T>());

            if (!inserted)
            {
                return nullptr;
            }

            IEntity *entity = iter->second.get();

            ((entity->register_component<C>()), ...);

            return entity;
        }

    private:
        std::unordered_map<std::string_view, std::unique_ptr<IEntity>> m_entities;
    };
}