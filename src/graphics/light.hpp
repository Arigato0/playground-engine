#pragma once
#include <glm/vec3.hpp>

namespace pge
{
    struct Light
    {
        bool is_active = true;
        glm::vec3 *position;

        bool is_spot ;
        glm::vec3 direction;
        float inner_cutoff = glm::cos(glm::radians(12.5f));
        float outer_cutoff = glm::cos(glm::radians(14.f));

        glm::vec3 color {1.0f};
        float ambient  {1.0f};
        float diffuse  {1.0f};
        float specular {0.1f};
        float power = 1;

        float constant  = 1.0f;
        float linear    = 0.09f;
        float quadratic = 0.032f;

        inline static std::vector<Light*> table;
    };
}
