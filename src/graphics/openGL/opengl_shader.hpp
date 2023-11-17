#pragma once

#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "opengl_error.hpp"
#include "../shaders.hpp"
#include "../../application/error.hpp"
#include "../../common_util/macros.hpp"

namespace pge
{
    class OpenGlShader : public IShader
    {
    public:
        ~OpenGlShader() override;

        uint32_t create(
            std::initializer_list<
            std::pair<const std::filesystem::path, ShaderType>> shaders) override;

        void use() override
        {
            glUseProgram(m_program);
        }

        void set(std::string_view name, int value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniform1i(location, value);
        }

        void set(std::string_view name, float value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniform1f(location, value);
        }

        void set(std::string_view name, glm::vec3 value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniform3f(location, EXPAND_VEC3(value));
        }

        void set(std::string_view name, glm::vec4 value) override
        {
            auto location = glGetUniformLocation(m_program, name.data());
            glUniform4f(location, EXPAND_VEC4(value));
        }

    private:
        uint32_t m_program;

        Result<uint32_t, OpenGlErrorCode> load_file(const std::filesystem::path &path, ShaderType type);
    };
}
