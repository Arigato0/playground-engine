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

        GlfwWindow()
        {
            if (!m_is_init)
            {
                glfwInit();
                m_is_init = true;
            }

            glfwSetErrorCallback(glfw_error_cb);
        }

        bool open(std::string_view title, int width, int height) override
        {
            if (!m_is_init)
            {
                glfwInit();
                m_is_init = true;
            }

            m_window = glfwCreateWindow(width, height, title.data(), glfwGetPrimaryMonitor(), nullptr);

            glfwMakeContextCurrent(m_window);

            set_raw_input(true);

            return m_window != nullptr;
        }

        void set_title(std::string_view title) override
        {
            glfwSetWindowTitle(m_window, title.data());
        }

        void resize(int width, int height) override
        {
            glfwSetWindowSize(m_window, width, height);
        }

        void change(int width, int height, int refresh_rate, int xpos = 0, int ypos = 0) override
        {
            glfwSetWindowMonitor(m_window, m_monitor, xpos, ypos, width, height, refresh_rate);
        }

        void cap_refresh_rate(bool value) override
        {
            glfwSwapInterval(value);
        }

        void set_fullscreen(bool value) override
        {
            auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            int xpos, ypos;
            glfwGetWindowPos(m_window, &xpos, &ypos);

            if (value && m_monitor == nullptr)
            {
                m_monitor = glfwGetPrimaryMonitor();
            }
            if (!value)
            {
                m_monitor = nullptr;
            }

            glfwSetWindowMonitor(m_window,
                m_monitor, xpos, ypos, mode->width, mode->height, mode->refreshRate);
        }

        void close() override
        {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        bool should_close() override
        {
            return glfwWindowShouldClose(m_window);
        }

        virtual void set_should_close(bool value) override
        {
            glfwSetWindowShouldClose(m_window, value);
        }

        ~GlfwWindow()
        {
            glfwTerminate();
            m_is_init = false;
            close();
        }

        void* handle() override
        {
            return m_window;
        }

        void resizable(bool value) override
        {
            glfwWindowHint(GLFW_RESIZABLE, value);
        }

        void set_graphics_api(GraphicsApi api) override
        {
            assert(is_implemented(api) && "unimplemented graphics api supplied");

            using enum GraphicsApi;

            switch (api)
            {
                case Vulkan:
                {
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                    break;
                }
                case OpenGl:
                {
                    glfwWindowHint(GLFW_SAMPLES, 8);
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
                    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                    break;
                }
            }
        }

        std::pair<uint32_t, uint32_t> framebuffer_size() override
        {
            uint32_t width, height;

            glfwGetFramebufferSize(m_window, (int*)&width, (int*)&height);

            return { width, height };
        }

        bool is_key_held(Key key) override
        {
            return glfwGetKey(m_window, (int)key) == GLFW_PRESS;
        }

        glm::dvec2 mouse_xy() override
        {
            glm::dvec2 output;

            glfwGetCursorPos(m_window, &output.x, &output.y);

            return output;
        }

        void set_cursor(CursorMode mode) override
        {
            glfwSetInputMode(m_window, GLFW_CURSOR, (int)mode);
        }

        virtual void set_raw_input(bool value) override
        {
            if (glfwRawMouseMotionSupported())
            {
                glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, value);
            }
        }

    private:
        GLFWmonitor *m_monitor = nullptr;
        GLFWwindow *m_window;
        bool m_is_init = false;

        static void glfw_error_cb(int code, const char* description)
        {
            Logger::warn("glfw error [{}]: {}", code, description);
        }
    };
}    

