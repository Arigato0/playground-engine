#pragma once

#include <string_view>
#include <cstdint>
#include <glm/vec2.hpp>

#include "input_keys.hpp"
#include "../graphics/graphics_api.hpp"

namespace pge
{
    class IWindow
    {
    public:
        virtual bool open(std::string_view title, int width, int height) = 0;

        virtual void set_title(std::string_view title) = 0;

        virtual void resize(int width, int height) = 0;

        virtual void close() = 0;

        virtual bool should_close() = 0;

        virtual void set_should_close(bool value) = 0;

        virtual void* handle() = 0;

        virtual void resizable(bool value) = 0;

        virtual void set_graphics_api(GraphicsApi api) = 0;

        virtual std::pair<uint32_t, uint32_t> framebuffer_size() = 0;

        virtual bool is_key_held(Key key) = 0;
        virtual glm::dvec2 mouse_xy() = 0;
        virtual void set_cursor(CursorMode mode) = 0;
        virtual void set_raw_input(bool value) = 0;
    };
}
