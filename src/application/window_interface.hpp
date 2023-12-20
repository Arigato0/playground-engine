#pragma once

#include <string_view>
#include <cstdint>
#include <glm/vec2.hpp>

#include "input_keys.hpp"
#include "../events/signal.hpp"
#include "../graphics/graphics_api.hpp"

namespace pge
{
    class IWindow
    {
    public:
        virtual ~IWindow() = default;

        virtual bool open(std::string_view title, int width, int height) = 0;

        virtual void set_title(std::string_view title) = 0;

        virtual void resize(int width, int height) = 0;

        virtual void change(int width, int height, int refresh_rate, int xpos, int ypos) = 0;

        virtual void set_fullscreen(bool value) = 0;

        virtual void cap_refresh_rate(bool value) = 0;

        virtual void close() = 0;

        virtual bool should_close() = 0;

        virtual void set_should_close(bool value) = 0;

        virtual void* handle() = 0;

        virtual void set_resizable(bool value) = 0;

        virtual bool is_fullscreen() const = 0;

        virtual void set_graphics_api(GraphicsApi api) = 0;

        virtual std::pair<uint32_t, uint32_t> framebuffer_size() = 0;

        virtual bool is_key_held(Key key) = 0;
        virtual glm::dvec2 mouse_xy() = 0;
        virtual void set_cursor(CursorMode mode) = 0;
        virtual void set_raw_input(bool value) = 0;

        // called when the window is requested to close
        Signal<void(IWindow*)> on_close;
        // called when the window is resized with its new width and height
        Signal<void(IWindow*, int, int)> on_resize;
        // called when the windows framebuffer is resized with its new width and height
        Signal<void(IWindow*, int, int)> on_framebuffer_resize;
        // called when the windows position changes with its new x and y positions
        Signal<void(IWindow*, int, int)> on_position;
        // called when the window is minimized into an icon second param is true if it was iconified otherwise it was restored
        Signal<void(IWindow*, bool)> on_iconify;
        // called when the window is maximized the second param is true if it was actually maximized otherwise it was restored
        Signal<void(IWindow*, bool)> on_maximize;

    };
}
