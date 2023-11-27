#pragma once

#include <string>
#include <memory>
#include <vector>

#include "transform.hpp"

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

    class IEntity;

    enum class EditorControlType
    {
        Slider,
        ColorEdit,
    };

    struct EditorProperty
    {
        std::string_view name;
        void *ptr;
        EditorControlType control_type;
    };

    class IComponent
    {
    public:
        virtual ~IComponent() = default;
        virtual void on_start() {}
        virtual void on_enable() {}
        virtual void on_disable() {}
        virtual void update(double delta_time) = 0;
        virtual std::string serialize() { return {}; }
        virtual void deserialize(std::string_view data) {}
        virtual std::vector<EditorProperty> editor_properties() { return {}; }

        void set_parent(IEntity *parent)
        {
            m_parent = parent;
        }

        bool is_enabled() const
        {
            return m_enabled;
        }

        void set_enabled(bool value)
        {
            m_enabled = value;

            if (m_enabled)
            {
                on_enable();
            }
            else
            {
                on_disable();
            }
        }

    protected:
        friend IEntity;
        IEntity *m_parent = nullptr;
        bool m_enabled = true;
    };

    template<class T>
    concept IsComponent = requires(T)
    {
        std::derived_from<IComponent, T>;
    };

    class IEntity
    {
    public:
        Transform transform;

        virtual ~IEntity() = default;
        virtual std::string serialize() { return {}; }
        virtual void deserialize(std::string_view data) {}

        void update_components(double delta_time)
        {
            for (auto &[_, component] : m_components)
            {
                if (component->m_enabled)
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
        std::unordered_map<std::string_view, std::unique_ptr<IComponent>> m_components;
    };

    template<class T>
    concept IsEntity = requires(T)
    {
        std::derived_from<IEntity, T>;
    };
}
