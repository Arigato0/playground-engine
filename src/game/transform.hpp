#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "glm/glm.hpp"

namespace pge
{
    struct Transform
    {
        glm::vec3 position {};
        glm::mat4 model {1.0f};

        void scale(glm::vec3 vec)
        {
            model = glm::scale(model, vec);
        }

        [[nodiscard]]
        glm::vec3 get_scale() const
        {
            return { model[0][0], model[1][1], model[2][2] };
        }

        void set_scale(glm::vec3 vec)
        {
            model[0][0] = vec.x;
            model[1][1] = vec.y;
            model[2][2] = vec.z;
        }

        [[nodiscard]]
        glm::vec3 get_position() const
        {
            return model[3];
        }

        void set_position(glm::vec3 vec)
        {
            position = vec;
            model[3] = {vec, 1.0f};
        }

        void translate(glm::vec3 vec)
        {
            position += vec;
            model = glm::translate(model, vec);
        }

        void rotate(float angle, glm::vec3 axis)
        {
            model = glm::rotate(model, glm::radians(angle), axis);
        }
    };
}