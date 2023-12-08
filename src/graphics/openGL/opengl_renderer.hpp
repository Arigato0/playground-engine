#pragma once

#include <list>
#include <set>

#include "opengl_shader.hpp"
#include "../renderer_interface.hpp"

namespace pge
{
    struct OpenGlMesh
    {
        uint32_t vbo;
        uint32_t vao;
        uint32_t ebo;

        uint32_t texture1;
        uint32_t texture2;

        uint32_t vertex_size;

        Material *material = nullptr;
    };

    class OpenglRenderer : public IRenderer
    {
    public:
        uint32_t init() override;

        IShader* create_shader(ShaderList shaders) override;

        size_t create_mesh(std::span<float> data, std::array<std::string_view, 2> textures) override;

        void set_material(Material *material, size_t mesh_id) override;

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
        std::vector<OpenGlMesh> m_meshes;
        OpenGlShader m_shader;
    };
}
