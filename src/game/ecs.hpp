#pragma once

#include <vector>
#include <string>
#include <glm/vec3.hpp>

namespace pge
{
#define PGE_CREATE_PROP_TABLE(prop_names...) \
    enum PROPERTY_NAME                  \
    {                                   \
        prop_names,                      \
        PROP_SIZE,                       \
    };                                  \
    std::array<std::any, PROP_SIZE> m_props; \
    template<class T> \
    T& get(int index) { return std::any_cast<T>(m_props[index]); } \

    // TODO implement this when you come up with the serialization scheme
#define PGE_MAKE_SERIALIZABLE() std::string serialize() override { return {}; } void deserialize(std::string_view data) override {}

    class Entity;

    class Component
    {
    public:
        bool should_update = true;

        virtual ~Component() = default;
        virtual void on_start() {}
        virtual void update(double delta_time) {}
        void set_parent(Entity *parent)
        {
            m_parent = parent;
        }
        // virtual std::string serialize() = 0;
        // virtual void deserialize(std::string_view data) = 0;
    protected:
        Entity *m_parent;
    };

    template<class T>
    concept IsComponent = requires(T)
    {
        std::derived_from<Component, T>;
    };

    class Entity
    {
    public:
        bool should_update = true;
        glm::vec3 transform;

        virtual ~Entity() = default;
        virtual void on_start() {}
        virtual void update(double delta_time) {}
        virtual std::string serialize() = 0;
        virtual void deserialize(std::string_view data) = 0;

        void update_components(double delta_time)
        {
            for (auto &[_, component] : m_components)
            {
                if (component->should_update)
                {
                    component->update(delta_time);
                }
            }
        }

        void start_components()
        {
            for (auto &[_, comp] : m_components)
            {
                comp->on_start();
            }
        }

        [[nodiscard]]
        uint32_t id() const
        {
            return m_id;
        }

        template<class T>
        T* find()
        {
            auto iter = m_components.find(typeid(T).name());

            if (iter == m_components.end())
            {
                return nullptr;
            }

            return (T*)iter->second.get();
        }

        template<IsComponent T>
        void register_component()
        {
            auto [comp, _] = m_components.emplace(typeid(T).name(), std::make_unique<T>());
            comp->second->set_parent(this);
        }

    protected:
        uint32_t m_id;
        std::unordered_map<std::string_view, std::unique_ptr<Component>> m_components;
    };

    template<class T>
    concept IsEntity = requires(T)
    {
        std::derived_from<Entity, T>;
    };
}