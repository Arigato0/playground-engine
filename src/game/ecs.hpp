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

    struct StartGroup {};

    struct EndGroup {};

#define SEPERATOR(name) {(name), SeperatorControl{} }
#define START_GROUP {"", StartGroup{} }
#define END_GROUP {"", EndGroup{} }

    using ButtonControl = std::function<void()>;

    struct EditorProperty;

    using EditorProperties = std::vector<EditorProperty>;

    using EditorControl = std::variant<
        bool*, Drag3Control<float>, DragControl<float>, ButtonControl, ColorEdit<float>,
        SeperatorControl, StartGroup, EndGroup>;

    struct EditorProperty
    {
        std::string_view name;
        EditorControl control;
    };

    class IComponent
    {
    public:
        virtual ~IComponent() = default;
        // will be called at the start of the scene. this garantuees all other components will exist and be initialized when this is called
        virtual void on_start() {}
        virtual void on_enable() {}
        virtual void on_disable() {}
        virtual void update(double delta_time) {}
        // on_init is called when a component is added to an entity
        virtual void on_init() {}
        // The editor_update is for adding functionality to a component so that it will only be run in the editors pause game view
        // This is particularly useful for mesh renderers that should render the mesh even while the game is not running in the editor
        virtual void editor_update(double delta_time) {}
        virtual std::string serialize() { return {}; }
        virtual void deserialize(std::string_view data) {}
        virtual EditorProperties editor_properties() { return {}; }

        virtual IComponent* clone() { return nullptr; }

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

#define PGE_COMPONENT(name) class name : public Component<name>
    // i dont like doing this but without modules my hands are tied
    void __proxy_register_comp__(std::string_view name, IComponent *comp);

    template<class T>
    class Component : public IComponent
    {
    public:
        Component()
        {
            __proxy_register_comp__(type_name<T>(), this);
        }

        IComponent* clone() override
        {
            auto ptr = new T();
            return ptr;
        }
    };

    template<class T>
    concept IsComponent = requires(T)
    {
        std::derived_from<IComponent, T>;
    };

    // creates an instance of the component to make sure it gets registered as a prototype
    template<IsComponent ...A>
    static void register_components()
    {
        (A(), ...);
    }

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

        IComponent* add_component_prototype(std::string_view name, IComponent *component)
        {
            auto [comp, _] = m_components.emplace(name, component->clone());

			auto *ptr = comp->second.get();

            init_comp(ptr);

			return ptr;
        }

        template<IsComponent T>
        void register_component()
        {
            auto [comp, _] = m_components.emplace(type_name<T>(), std::make_unique<T>());
            init_comp(comp->second.get());
        }

        void remove_component(std::string_view name)
        {
            m_components.erase(name);
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

        void init_comp(IComponent *comp)
        {
            comp->set_parent(this);
            comp->on_init();
        }
    };
}
