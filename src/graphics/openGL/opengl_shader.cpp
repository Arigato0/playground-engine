#include "opengl_shader.hpp"

#include <vector>
#include <application/platform/fs_events.hpp>

#include "common_util/io.hpp"
#include "application/log.hpp"
#include "application/engine.hpp"

constexpr int LOG_SIZE = 512;
static char g_info_log[LOG_SIZE];

inline uint32_t opengl_shader_type(pge::ShaderType type)
{
    using enum pge::ShaderType;

    switch (type)
    {
        case Vertex: return GL_VERTEX_SHADER;
        case Fragment: return GL_FRAGMENT_SHADER;
		case Geometry: return GL_GEOMETRY_SHADER;
    }
}

pge::Result<unsigned, pge::OpenGlErrorCode> load_file(const std::filesystem::path &path, pge::ShaderType type)
{
    auto contents = util::read_file(path);

    if (contents.empty())
    {
		Logger::info("contents empty {}", path);
        return pge::OPENGL_ERROR_SHADER_CREATION;
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

        return pge::OPENGL_ERROR_SHADER_CREATION;
    }

    return id;
}

uint32_t make_program(pge::ShaderList shaders, uint32_t &out_program)
{
	std::array<uint32_t, pge::MAX_SHADERS_TYPES> cleanup {};

    auto program = glCreateProgram();

    if (program == 0)
    {
        return pge::OPENGL_ERROR_SHADER_CREATION;
    }

	uint32_t shaders_created = 0;

    for (const auto &[path, type] : shaders)
    {
        auto result = load_file(path, type);

        if (!result.ok())
        {
            return result.error();
        }

        glAttachShader(program, *result);

		cleanup[shaders_created] = result.get();
		shaders_created++;
    }

    glLinkProgram(program);

	for (int i = 0; i < shaders_created; i++)
    {
        glDeleteShader(cleanup[i]);
    }

	out_program = program;

	return pge::OPENGL_ERROR_OK;
}

pge::GlShader::~GlShader()
{
    glDeleteProgram(m_program);
}

uint32_t pge::GlShader::create(pge::ShaderList shaders)
{
	if (m_monitors[0] == -1)
	{
		for (auto i = 0; auto &[path, _] : shaders)
		{
			m_monitors[i] = Engine::fs_monitor.add_watch(path.c_str(), FSE_MODIFY,
			[&program = m_program, shaders](int mask, std::string_view _)
			{
				glDeleteProgram(program);
				make_program(shaders, program);
			});

			i++;
		}
	}

    return make_program(shaders, m_program);
}