#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "light.hpp"

namespace pge
{
    struct Texture
    {
        uint32_t id;
        int scale = 1;
        float mix = 0.0f;
        bool enabled = true;
    };

    struct Material
    {
        float shininess = 32;
        Texture diffuse_texture;
        Texture specular_texture;
        glm::vec3 specular {1.0};
        glm::vec3 color {0.0f};
        std::vector<Light*> lights;
    };

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normals;
        glm::vec2 uv;
    };

    struct Mesh
    {
        uint32_t id;
        std::vector<Vertex> vertices;
        std::vector<Texture> textures;
    };
}
