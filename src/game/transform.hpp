#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "glm/glm.hpp"

namespace pge
{
    struct Transform
    {
        glm::vec3 position;
        float     rotation;
        glm::mat4 transform {1.0f};

        void scale(glm::vec3 vec)
        {
            transform = glm::scale(transform, vec);
        }

        void translate(glm::vec3 vec)
        {
            transform = glm::translate(transform, vec);
        }

        void rotate(float angle, glm::vec3 axis)
        {
            transform = glm::rotate(transform, glm::radians(angle), axis);
            rotation += angle;
        }
    };
}