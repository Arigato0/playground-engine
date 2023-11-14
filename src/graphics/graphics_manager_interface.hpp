#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "../application/window_interface.hpp"
#include "../application/fmt.hpp"
#include "graphics_api.hpp"
#include "types.hpp"

namespace pge
{
    // the properties of the system and graphics library used
    struct GraphicsProperties
    {
        std::string device_name;
        uint32_t    version_major;
        uint32_t    version_minor;
        GraphicsApi api;

        std::string to_string()
        {
            return fmt::format("[{}:{}.{}] {}",
                pge::to_string(api),
                version_major,
                version_minor,
                device_name);
        }
    };

    // A graphics subsystem responsible for resource management of the underlying graphics library
    class IGraphicsManager
    {
    public:
        virtual ~IGraphicsManager() = default;
        virtual uint8_t init() = 0;
        virtual void set_window(IWindow *window) = 0;
        virtual uint8_t draw_frame() = 0;
        virtual void wait() = 0;
        virtual void set_clear_color(glm::vec4 values) = 0;
        virtual glm::vec4 get_clear_color() = 0;
        virtual void draw_wireframe(bool value) = 0;
        virtual std::vector<const char*> extensions() = 0;
        virtual GraphicsProperties properties() = 0;
        virtual std::string_view error_message(uint8_t code) = 0;
    };
}
