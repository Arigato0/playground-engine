#pragma once

#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_error.hpp"
#include "../shader_interface.hpp"
#include "../../application/log.hpp"
#include "../../application/error.hpp"
#include "../../common_util/macros.hpp"
#include "data/hash_table.hpp"

#include <glad/glad.h>
#include <common_util/misc.hpp>

namespace pge
{
	void set_uniform(uint32_t program, std::string_view name, const pge::UniformValue &value);

    class GlShader : public IShader
    {
    public:
        ~GlShader() override;

        uint32_t create(ShaderList shaders) override;

        IShader& use() override
        {
            glUseProgram(m_program);
			return *this;
        }

        IShader& set(std::string_view name, int value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniform1i(location, value);
			m_cache[name] = value;
			return *this;
        }

        IShader& set(std::string_view name, float value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniform1f(location, value);
			m_cache[name] = value;
			return *this;
        }

        IShader& set(std::string_view name, glm::vec2 value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniform2f(location, EXPAND_VEC2(value));
			m_cache[name] = value;
			return *this;
        }

        IShader& set(std::string_view name, glm::vec3 value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniform3f(location, EXPAND_VEC3(value));
			m_cache[name] = value;
			return *this;
        }

        IShader& set(std::string_view name, glm::vec4 value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniform4f(location, EXPAND_VEC4(value));
			m_cache[name] = value;
			return *this;
        }

        IShader& set(std::string_view name, glm::mat4 value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
			m_cache[name] = value;
			return *this;
        }

    private:
        uint32_t m_program = -1;
		int m_count = 0;
		std::array<int, MAX_SHADERS_TYPES> m_monitors = {-1};
		// a cache for uniforms used to set uniforms to their previous state when shaders reload
		HashMap<std::string, UniformValue, ENABLE_TRANSPARENT_HASH> m_cache;
    };
}

