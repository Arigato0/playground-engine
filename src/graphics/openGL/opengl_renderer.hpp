#pragma once

#include <list>
#include <set>

#include "opengl_shader.hpp"
#include "../renderer_interface.hpp"

namespace pge
{
    struct GlMesh
    {
        uint32_t vbo;
        uint32_t vao;
        uint32_t ebo;

        const Mesh *data = nullptr;
    };

    class OpenglRenderer : public IRenderer
    {
    public:
        uint32_t init() override;

        IShader* create_shader(ShaderList shaders) override;

        size_t create_mesh(const Mesh &mesh) override;

        void new_frame() override;

        uint32_t draw(size_t mesh_id, glm::mat4 transform) override;

        uint32_t create_texture(std::string_view path, uint32_t &out_texture) override;

        void delete_texture(uint32_t id) override;

        void set_wireframe_mode(bool value) override
        {
            glPolygonMode(GL_FRONT_AND_BACK, value ? GL_LINE : GL_FILL);
        }

        void wait() override
        {
            glFinish();
        }

        std::string_view error_message(uint32_t code) override
        {
            return opengl_error_message((OpenGlErrorCode)code);
        }

        RendererProperties properties() override;

    private:
        uint32_t m_missing_texture;
        std::vector<GlMesh> m_meshes;
        GlShader m_shader;
    };
}
