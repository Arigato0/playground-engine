#pragma once

#define PGE_SHADER_PATH "./src/shaders/"
#define PGE_FIND_SHADER(s) PGE_SHADER_PATH s

namespace pge
{
    enum class ShaderType
    {
        Vertex,
        Fragment,
    };
}