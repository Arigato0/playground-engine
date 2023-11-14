#include "opengl_shader.hpp"

#include "../../common_util/io.hpp"
#include "../../application/log.hpp"

#include "glad/glad.h"

constexpr int LOG_SIZE = 512;
static char g_info_log[LOG_SIZE];

inline uint32_t opengl_shader_type(pge::ShaderType type)
{
    using enum pge::ShaderType;

    switch (type)
    {
        case Vertex: return GL_VERTEX_SHADER;
        case Fragment: return GL_FRAGMENT_SHADER;
    }
}

pge::Result<uint32_t, pge::OpenGlErrorCode> pge::create_opengl_shader(const std::filesystem::path& path, ShaderType type)
{
    auto shader_type = opengl_shader_type(type);

    auto id = glCreateShader(shader_type);

    auto contents = util::read_file(path);

    if (contents.empty())
    {
        return OPENGL_ERROR_SHADER_CREATION;
    }

    auto data = contents.c_str();
    glShaderSource(id, 1, (const GLchar* const*)&data, nullptr);
    glCompileShader(id);

    int ok;

    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);

    if (!ok)
    {
        glGetShaderInfoLog(id, LOG_SIZE, nullptr, g_info_log);

        Logger::info("Error compiling opengl shader. {}", g_info_log);

        return OPENGL_ERROR_SHADER_CREATION;
    }

    return id;
}
