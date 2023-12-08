#pragma once
#include <glm/vec3.hpp>

namespace pge
{
    struct Light
    {
        glm::vec3 *position;

        bool is_dir;
        glm::vec3 direction;
        float inner_cutoff = glm::cos(glm::radians(12.5f));
        float outer_cutoff = glm::cos(glm::radians(14.f));

        glm::vec3 ambient  {1.0f};
        glm::vec3 diffuse  {1.0f};
        glm::vec3 specular {0.1f};

        float constant  = 1.0f;
        float linear    = 0.09f;
        float quadratic = 0.032f;

        inline static std::vector<Light*> table;
    };
}
