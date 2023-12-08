#pragma once
#include <glm/vec3.hpp>

namespace pge
{
    struct Light
    {
        glm::vec3 position;

        bool is_dir;
        glm::vec3 direction;
        float inner_cutoff;
        float outer_cutoff;

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;

        float constant;
        float linear;
        float quadratic;
    };
}
