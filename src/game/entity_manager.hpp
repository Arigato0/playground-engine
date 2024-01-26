#pragma once

#include <algorithm>
#include <unordered_map>

#include "ecs.hpp"
#include "common_util/misc.hpp"
#include "data/hash_table.hpp"

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
        Entity* create(std::string_view name)
        {
            auto [iter, inserted] = m_entities.emplace(name, Entity{});

            if (!inserted)
            {
                return nullptr;
            }

            Entity &entity = iter->second;

            entity.m_name = iter->first;

            ((entity.register_component<C>()), ...);

            return &entity;
        }

        bool register_prototype(std::string_view name, IComponent *component)
        {
            if (m_comp_prototypes.contains(name))
            {
                return false;
            }

            auto [iter, created] = m_comp_prototypes.emplace(name, nullptr);

            // sets the pointer later otherwise it will get stuck in recursion
            iter->second.reset(component->clone());

            return created;
        }

        template<class T>
        bool register_prototype(const T &component)
        {
            auto name = type_name<T>();

            if (m_comp_prototypes.contains(name))
            {
                return false;
            }

            auto [_, created] = m_comp_prototypes.emplace(name, std::make_unique<T>(component));

            return created;
        }

        template<class T>
        IComponent* create_from_prototype()
        {
            auto name = type_name<T>();

            auto iter = m_comp_prototypes.find(name);

            if (iter == m_comp_prototypes.end())
            {
                return nullptr;
            }

            return iter->second->clone();
        }

        void erase(std::string_view name)
        {
//			m_entities.erase(name);
            auto iter = m_entities.find(name);
            m_entities.erase(iter);
        }

        void clear()
        {
            m_entities.clear();
        }

        EntityTable& get_entities()
        {
            return m_entities;
        }

        Entity::ComponentTable& get_comp_prototypes()
        {
            return m_comp_prototypes;
        }

        std::vector<std::string_view> get_comp_protype_names()
        {
            std::vector<std::string_view> output;

            output.reserve(m_comp_prototypes.size());

            for (auto &[name, _] : m_comp_prototypes)
            {
                output.push_back(name);
            }

            return output;
        }

    private:
        EntityTable m_entities;
        Entity::ComponentTable m_comp_prototypes;
    };
}