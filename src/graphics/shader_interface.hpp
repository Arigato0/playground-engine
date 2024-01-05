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
		Geometry,
    };

    using ShaderList = std::initializer_list<
            std::pair<const std::filesystem::path, ShaderType>>;

    class IShader
    {
    public:
        virtual ~IShader() = default;
        virtual uint32_t create(ShaderList shaders) = 0;
        virtual IShader& use() = 0;
        virtual IShader& set(std::string_view name, int value) = 0;
        virtual IShader& set(std::string_view name, float value) = 0;
        virtual IShader& set(std::string_view name, glm::vec2 value) = 0;
        virtual IShader& set(std::string_view name, glm::vec3 value) = 0;
        virtual IShader& set(std::string_view name, glm::vec4 value) = 0;
        virtual IShader& set(std::string_view name, glm::mat4 value) = 0;
    };
}