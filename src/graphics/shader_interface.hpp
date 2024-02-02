#pragma once

#include "glm/glm.hpp"
#include <utility>
#include <filesystem>
#include <initializer_list>
#include <array>
#include <variant>

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

	constexpr int MAX_SHADERS_TYPES = 3;

	using UniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;
	using ShaderPath = std::pair<std::filesystem::path, ShaderType>;

	class ShaderList
	{
	public:

		ShaderList(std::initializer_list<ShaderPath> list) :
			m_count(list.size())
		{
			for (auto i = 0; auto &path : list)
			{
				m_paths[i] = path;
				i++;
			}
		}

		ShaderPath* begin()
		{
			return m_paths.begin();
		}

		ShaderPath* end()
		{
			return &m_paths[m_count];
		}

		[[nodiscard]]
		uint32_t count() const
		{
			return m_count;
		}

	private:
		uint32_t m_count = 0;
		std::array<ShaderPath, MAX_SHADERS_TYPES> m_paths;
	};

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
//		virtual IShader& set(std::string_view name, const UniformValue &&value) = 0;
    };
}