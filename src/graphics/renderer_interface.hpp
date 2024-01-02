#pragma once
#include <cstdint>
#include <span>

#include <glm/glm.hpp>

#include "Camera.hpp"
#include "Camera.hpp"
#include "graphics_api.hpp"
#include "model.hpp"
#include "shaders.hpp"
#include "../application/fmt.hpp"
#include "Camera.hpp"
#include "framebuffer_interface.hpp"
#include "render_options.hpp"
#include "../data/string.hpp"
#include "render_view.hpp"

namespace pge
{
    struct RendererProperties
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

    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

        // initializes the graphics subsystem
        virtual uint32_t init() = 0;

        // creates a shader program using the underlying graphics api
        virtual IShader* create_shader(ShaderList shaders) = 0;

        virtual void create_buffers(Mesh &mesh) = 0;

        virtual void delete_buffers(Mesh &mesh) = 0;

        virtual void set_visualize_depth(bool value) = 0;

        virtual void new_frame() = 0;

        virtual void end_frame() = 0;

        // draws the given mesh
        virtual void draw(const MeshView&mesh, glm::mat4 transform, DrawOptions options = {}) = 0;

        // sets if wireframe mode is active
        virtual void set_wireframe_mode(bool value) = 0;

        virtual void set_clear_color(glm::vec4 value) = 0;

        virtual glm::vec4 get_clear_color() = 0;

        // if set true all renders will become offline and the result will have to be accessed via the framebuffer
        virtual void set_offline(bool value) = 0;

        // waits for all render commands to finish
        virtual void wait() = 0;

        // gets underlying renderer properties
        virtual RendererProperties properties() = 0;

        // translates the underlying graphics apis error code to a string
        virtual std::string_view error_message(uint32_t code) = 0;

        virtual uint32_t create_texture_from_path(std::string_view path, uint32_t &out_texture, TextureOptions options) = 0;

        virtual uint32_t create_texture(ustring_view data, int width, int height, int channels, uint32_t &out_texture,
			TextureWrapMode wrap_mode, bool gamma_correct) = 0;

        virtual uint32_t create_cubemap_from_path(std::array<std::string_view, 6> faces, uint32_t &out_texture) = 0;

        virtual Image get_image() = 0;

        virtual void delete_texture(uint32_t id) = 0;

        virtual void set_skybox(uint32_t id) = 0;

        void set_camera(Camera *camera)
        {
            m_camera = camera;
        }

		virtual RenderView* add_view(Camera *camera) = 0;

		virtual void remove_view(RenderView *view) = 0;

        virtual IFramebuffer* get_framebuffer() = 0;

		virtual IFramebuffer* get_render_framebuffer() = 0;

		virtual void set_gamma(float value) = 0;

    protected:
        // the main camera that will be used for renders.
        Camera *m_camera;
    };
}
