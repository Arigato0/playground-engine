#pragma once
#include <cstdint>
#include <span>

#include <glm/glm.hpp>

#include "graphics_api.hpp"
#include "model.hpp"
#include "shaders.hpp"
#include "../application/fmt.hpp"
#include "CameraData.hpp"

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

        // initilizes the graphics subsystem
        virtual uint32_t init() = 0;

        // creates a shader program using the underlying graphics api
        virtual IShader* create_shader(ShaderList shaders) = 0;

        virtual void create_buffers(Mesh &mesh) = 0;

        virtual void delete_buffers(Mesh &mesh) = 0;

        virtual void new_frame() = 0;

        // draws the given mesh
        virtual uint32_t draw(const Mesh &mesh, glm::mat4 transform) = 0;

        // sets if wireframe mode is active
        virtual void set_wireframe_mode(bool value) = 0;

        // waits for all render commands to finish
        virtual void wait() = 0;

        // gets underlying renderer properties
        virtual RendererProperties properties() = 0;

        // translates the underlying graphics apis error code to a string
        virtual std::string_view error_message(uint32_t code) = 0;

        virtual uint32_t create_texture(std::string_view path, uint32_t &out_texture) = 0;

        virtual void delete_texture(uint32_t id) = 0;

        void set_camera(CameraData *camera)
        {
            m_camera = camera;
        }

        glm::vec4 clear_color;

    protected:
        CameraData *m_camera;
    };
}
