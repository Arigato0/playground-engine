#pragma once

#include <filesystem>

#include "opengl_error.hpp"
#include "../shaders.hpp"
#include "../../application/error.hpp"

namespace pge
{
    Result<uint32_t, OpenGlErrorCode> create_opengl_shader(const std::filesystem::path&path, ShaderType type);

}
