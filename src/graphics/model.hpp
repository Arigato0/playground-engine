#pragma once

#include <vector>
#include <string_view>
#include <string>
#include <glm/glm.hpp>

namespace pge
{
    struct Texture
    {
        uint32_t id;
        float scale = 1;
        bool enabled = true;

        Texture() = default;
        void set(std::string_view path);
        Texture(std::string_view path);
        ~Texture();
    };

    struct Material
    {
        float shininess = 32;
        Texture diffuse;
        Texture specular;
        glm::vec3 color {0.0f};
        bool recieve_lighting = true;
    };

    struct Vertex
    {
        glm::vec3 position {};
        glm::vec3 normal   {};
        glm::vec2 coord    {};
    };

    struct Mesh
    {
        uint32_t id;
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        Material material {};
    };

    struct Model
    {
        uint32_t id;
        std::vector<Mesh> meshes;
    };
}
