#include "opengl_shader.hpp"

#include <vector>

#include "../../common_util/io.hpp"
#include "../../application/log.hpp"

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

pge::GlShader::~GlShader()
{
    glDeleteProgram(m_program);
}

uint32_t pge::GlShader::create(std::initializer_list<std::pair<const std::filesystem::path, ShaderType>> shaders)
{
    std::vector<uint32_t> cleanup;

    cleanup.reserve(shaders.size());

    m_program = glCreateProgram();

    if (m_program == 0)
    {
        return OPENGL_ERROR_SHADER_CREATION;
    }

    for (auto [path, type] : shaders)
    {
        auto result = load_file(path, type);

        if (!result.ok())
        {
            return result.error();
        }

        glAttachShader(m_program, *result);
        cleanup.push_back(*result);
    }

    glLinkProgram(m_program);

    for (const auto shader : cleanup)
    {
        glDeleteShader(shader);
    }

    return OPENGL_ERROR_OK;
}

pge::Result<unsigned, pge::OpenGlErrorCode> pge::GlShader::load_file(const std::filesystem::path& path,
    ShaderType type)
{
    auto contents = util::read_file(path);

    if (contents.empty())
    {
        return OPENGL_ERROR_SHADER_CREATION;
    }

    auto data = contents.c_str();

    auto shader_type = opengl_shader_type(type);
    auto id = glCreateShader(shader_type);

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
