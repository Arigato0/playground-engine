#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "light.hpp"

namespace pge
{
    struct Texture
    {
        uint32_t id;
        float scale = 1;
        bool enabled = true;
    };

    struct Material
    {
        float shininess = 128;
        Texture diffuse_texture;
        Texture specular_texture;
        glm::vec3 specular {0.1};
        glm::vec3 color {0.0f};
        bool recieve_lighting = true;
    };

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    struct Mesh
    {
        uint32_t id;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Texture> textures;
    };
}
