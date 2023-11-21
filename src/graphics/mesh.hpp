#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace pge
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv_cordinate;
    };

    class Mesh
    {
    public:
        static constexpr size_t MAX_TEXTURES = 16;
    private:
        std::vector<Vertex> m_vertices;
        std::array<uint32_t, MAX_TEXTURES> m_textures;
    };
}