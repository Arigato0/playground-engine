#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "renderer_interface.hpp"
#include "../application/engine.hpp"
#include "shaders.hpp"
#include "../game/ecs.hpp"
#include "glm/glm.hpp"

namespace pge
{
    class Camera
    {
    public:
        float fov = 65;
        float yaw = -90;
        float pitch;
        float roll;

        float near = 0.1;
        float far = 100.f;

        glm::mat4 projection {1.0f};
        glm::mat4 view {1.0f};

        glm::vec3 position;
        glm::vec3 up {0, 1, 0};
        glm::vec3 right;
        glm::vec3 forward {0, 0, -1};

        Camera()
        {
            m_shader = Engine::renderer->get_shader();
        }

        void update()
        {
            static glm::vec3 direction {};

            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

            forward = glm::normalize(direction);

            auto [window_width, window_height] = Engine::window.framebuffer_size();

            //view = glm::lookAt(position, {0, 0, -1}, {0, 1, 0});
            view = glm::lookAt(position, position + forward, up);
            projection = glm::perspective(glm::radians(fov), (float)(window_width / window_height), near, far);

            m_shader->set("projection", projection);
            m_shader->set("view", view);
        }

    private:
        IShader *m_shader;
    };
}