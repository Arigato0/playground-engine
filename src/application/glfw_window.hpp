#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string_view>
#include <cassert>
#include "fmt.hpp"
#include "log.hpp"

#include "../graphics/graphics_api.hpp"
#include "window_interface.hpp"

#define PGE_PLATFORM_WINDOW_HANDLE GLFWwindow*

namespace pge
{
    class GlfwWindow : public IWindow
    {
    public:
        GlfwWindow();

        bool open(std::string_view title, int width, int height) override;

        void set_title(std::string_view title) override;

        void resize(int width, int height) override;

        void change(int width, int height, int refresh_rate, int xpos = 0, int ypos = 0) override;

        void cap_refresh_rate(bool value) override;

        void set_fullscreen(bool value) override;

        void close() override;

        bool should_close() override;

        void set_should_close(bool value) override;

        ~GlfwWindow() override;

        void* handle() override;

        void set_resizable(bool value) override;

        void set_graphics_api(GraphicsApi api) override;

        std::pair<uint32_t, uint32_t> framebuffer_size() override;

        bool is_key_held(Key key) override;

        glm::dvec2 mouse_xy() override;

        void set_cursor(CursorMode mode) override;

        void set_raw_input(bool value) override;

        bool is_fullscreen() const override;

    private:
        GLFWmonitor *m_monitor = nullptr;
        GLFWwindow *m_window;
        int m_width;
        int m_height;
        bool m_is_init = false;

        static void glfw_error_cb(int code, const char* description);

        void set_callbacks();
    };
}    

