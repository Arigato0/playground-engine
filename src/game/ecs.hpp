#pragma once

#include <functional>
#include <string>
#include <memory>
#include <variant>
#include <vector>

#include "transform.hpp"
#include "../application/log.hpp"
#include "../common_util/misc.hpp"

namespace pge
{
    class Entity;

    template<class T>
    struct RangeControl
    {
        T min;
        T max;
    };

    template<class T>
    struct Drag3Control
    {
        T *value;

        float speed = 1;
        float min = 0;
        float max = 0;
        const char* format = "%.3f";
    };

    template<class T>
    struct DragControl
    {
        T *value;

        float speed = 1;
        float min = 0;
        float max = 0;
        const char* format = "%.3f";
    };

    template<class T>
    struct ColorEdit
    {
        T *value;
    };

    struct SeperatorControl
    {};

#define SEPERATOR(name) {(name), SeperatorControl{}}

    using ButtonControl = std::function<void()>;

    struct EditorProperty;

    using EditorProperties = std::vector<EditorProperty>;

    using EditorControl = std::variant<
        bool*, Drag3Control<float>, DragControl<float>, ButtonControl, ColorEdit<float>,
        SeperatorControl>;

    struct EditorProperty
    {
        std::string_view name;
        EditorControl control;
    };

    class IComponent
    {
    public:
        virtual ~IComponent() = default;
        virtual void on_start() {}
        virtual void on_enable() {}
        virtual void on_disable() {}
        virtual void update(double delta_time) {}
        virtual std::string serialize() { return {}; }
        virtual void deserialize(std::string_view data) {}
        virtual EditorProperties editor_properties() { return {}; }

        void set_parent(Entity *parent)
        {
            m_parent = parent;
        }

        [[nodiscard]]
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
        friend Entity;
        Entity *m_parent = nullptr;
        bool m_enabled = true;
    };

    template<class T>
    concept IsComponent = requires(T)
    {
        std::derived_from<IComponent, T>;
    };

    class EntityManager;

    class Entity
    {
    public:
        using ComponentTable = std::unordered_map<std::string_view, std::unique_ptr<IComponent>>;
        Transform transform;

        Entity() = default;

        Entity(std::string_view name) :
            m_name(name)
        {}

        ~Entity() = default;

        Entity(Entity &&other) noexcept :
            m_id(other.m_id),
            m_components(std::move(other.m_components)),
            m_name(other.m_name)
        {}

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

        std::string_view name() const
        {
            return m_name;
        }

        [[nodiscard]]
        uint32_t id() const
        {
            return m_id;
        }

        template<class T>
        T* find()
        {
            auto iter = m_components.find(type_name<T>());

            if (iter == m_components.end())
            {
                return nullptr;
            }

            return (T*)iter->second.get();
        }

        template<IsComponent T>
        void register_component()
        {
            auto [comp, _] = m_components.emplace(type_name<T>(), std::make_unique<T>());
            comp->second->set_parent(this);
        }

        ComponentTable& get_components()
        {
            return m_components;
        }

    private:
        friend EntityManager;
        uint32_t m_id;
        ComponentTable m_components;
        std::string_view m_name;
    };
}
