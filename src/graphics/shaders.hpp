#pragma once

#include "glm/glm.hpp"
#include <utility>
#include <filesystem>
#include <initializer_list>
#define PGE_SHADER_PATH "./src/shaders/"
#define PGE_FIND_SHADER(s) PGE_SHADER_PATH s

namespace pge
{
    enum class ShaderType
    {
        Vertex,
        Fragment,
    };

    class IShader
    {
    public:
        virtual ~IShader() = default;
        virtual uint32_t create(
            std::initializer_list<
            std::pair<const std::filesystem::path, ShaderType>> shaders) = 0;
        virtual void use() = 0;
        virtual void set(std::string_view name, int value) = 0;
        virtual void set(std::string_view name, float value) = 0;
        virtual void set(std::string_view name, glm::vec3 value) = 0;
        virtual void set(std::string_view name, glm::vec4 value) = 0;
    };
}