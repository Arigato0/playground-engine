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
    class OpenglRenderer : public IRenderer
    {
        struct GlBuffers
        {
            GLuint vbo;
            GLuint vao;
            GLuint ebo;
        };

    public:
        uint32_t init() override;

        IShader* create_shader(ShaderList shaders) override;

        void create_buffers(Mesh &mesh) override;

        void delete_buffers(Mesh &mesh) override;

        void set_visualize_depth(bool value) override;

        void new_frame() override;

        void end_frame() override;

        void draw(const MeshView&mesh, glm::mat4 model, DrawOptions options = {}) override;

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
        // the default missing texture to use when unable to create a texture
        uint32_t m_missing_texture;
        // the opengl buffers every mesh needs to be drawn.
        IdTable<GlBuffers> m_buffers;
        // the base shader
        GlShader m_shader;
        // a shader used for outlinening
        GlShader m_outline_shader;
        GlShader m_screen_shader;
        // the screen plane where framebuffer textures are drawn to
        GlBuffers m_screen_plane;
        // possibly transparent meshes that are sorted by their distance from the camera
        std::multimap<float, DrawData> m_sorted_meshes;
        // meshes that are queued for drawing in all render passes
        std::vector<DrawData> m_render_queue;
        // the shaders that will be used for different render passes such as post processing stuff
        std::list<GlShader> m_shaders;
        // the queue for the buffers that are supposed to be deleted
        std::vector<uint32_t> m_delete_queue;
        GlFramebuffer m_framebuffer;

        void draw_shaded_wireframe(const Mesh &mesh, glm::mat4 model);

        void handle_lighting();

        uint32_t handle_draw(const DrawData&data);

        void draw_passes();

        void draw_everything();

        void clear_buffers();

        void draw_outline(const DrawData &data);

        void handle_gl_buffer_delete();

        void set_base_uniforms(const DrawData &data);

        void set_vert_uniforms(const DrawData &data);

        void create_screen_pane();
    };
}
