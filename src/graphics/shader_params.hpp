#pragma once

#include <glm/glm.hpp>

namespace pge
{
    // all the required information to pass to shaders
    struct ShaderParams
    {
        bool textures_enabled = true;
        bool color_enabled = false;
        glm::vec3 object_color {1.0f};
        glm::vec3 light_color {1.0f};
        float texture_mix = 0.0f;
        IShader *shader = nullptr;
        glm::vec3 *light_source = nullptr;
    };
}