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

    class Entity
    {
    public:
        glm::vec3 transform;

        virtual ~Entity() = default;
        virtual void update(double delta_time) = 0;
        virtual std::string serialize() = 0;
        virtual void deserialize(std::string_view data) = 0;

        [[nodiscard]]
        uint32_t id() const
        {
            return m_id;
        }

    private:
        uint32_t m_id;
    };
}
