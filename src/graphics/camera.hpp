#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "glm/glm.hpp"

namespace pge
{
    struct Camera
    {
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

        void update();

    };
}