#pragma once

#include <imgui.h>

#include "glad/glad.h"
#include "../graphics_manager_interface.hpp"
#include "opengl_error.hpp"
#include "opengl_shader.hpp"

namespace pge
{
    class OpenGlManager : public IGraphicsManager
    {
    public:

        uint8_t init() override;

        uint8_t draw_frame() override;

        inline void set_window(IWindow *window) override
        {
            m_window = window;
        }

        inline void wait() override
        {
            glFinish();
        }

        inline std::string_view error_message(uint8_t error) override
        {
            return opengl_error_message((OpenGlErrorCode)error);
        }

        inline GraphicsProperties properties() override
        {
            GraphicsProperties output
            {
                .device_name = (const char*)glGetString(GL_RENDERER),
                .api = GraphicsApi::OpenGl
            };

            glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&output.version_major);
            glGetIntegerv(GL_MINOR_VERSION, (GLint*)&output.version_minor);

            return output;
        }

        void set_clear_color(glm::vec4 value) override
        {
            m_clear_color = value;
        }

        virtual glm::vec4 get_clear_color() override
        {
            return m_clear_color;
        }

        void draw_wireframe(bool value) override
        {
            glPolygonMode(GL_FRONT_AND_BACK, value ? GL_LINE : GL_FILL);
        }

        std::vector<const char*> extensions() override;

    private:
        IWindow *m_window;
        glm::vec4 m_clear_color;
        OpenGlShader m_shader;
        uint32_t m_vao;
        uint32_t m_texture;
        uint32_t m_texture2;
    };
}
