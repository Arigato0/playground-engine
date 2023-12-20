#pragma once

#include <list>
#include <map>
#include <set>

#include "gl_framebuffer.hpp"
#include "opengl_shader.hpp"
#include "../renderer_interface.hpp"
#include "../render_options.hpp"
#include "../../data/id_table.hpp"
#include "../../data/string.hpp"

namespace pge
{
    struct GlBuffers
    {
        GLuint vbo;
        GLuint vao;
        GLuint ebo;
    };

    class OpenglRenderer : public IRenderer
    {
    public:
        uint32_t init() override;

        IShader* create_shader(ShaderList shaders) override;

        void create_buffers(Mesh &mesh) override;

        void delete_buffers(Mesh &mesh) override;

        void set_visualize_depth(bool value) override;

        void new_frame() override;

        void end_frame() override;

        uint32_t draw(const MeshView &mesh, glm::mat4 model, DrawOptions options = {}) override;

        uint32_t create_texture_from_path(std::string_view path, uint32_t &out_texture,
            bool flip, TextureWrapMode wrap_mode) override;

        uint32_t create_texture(ustring_view data, int width, int height, int channels,
            uint32_t &out_texture, TextureWrapMode wrap_mode) override;

        void delete_texture(uint32_t id) override;

        void set_wireframe_mode(bool value) override
        {
            glPolygonMode(GL_FRONT_AND_BACK, value ? GL_LINE : GL_FILL);
        }

        void wait() override
        {
            glFinish();
        }

        IFramebuffer* get_framebuffer() override
        {
            return &m_framebuffer;
        }

        std::string_view error_message(uint32_t code) override
        {
            return opengl_error_message((OpenGlErrorCode)code);
        }

        RendererProperties properties() override;

    private:
        uint32_t m_missing_texture;
        IdTable<GlBuffers> m_buffers;
        GlShader m_shader;
        GlShader m_outline_shader;
        std::multimap<float, DrawData> m_sorted_meshes;
        GlFramebuffer m_framebuffer;

        void draw_shaded_wireframe(const Mesh &mesh, glm::mat4 model);

        void handle_lighting();

        unsigned handle_draw(const MeshView&mesh, glm::mat4 model, DrawOptions options);
    };
}
