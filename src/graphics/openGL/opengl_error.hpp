#pragma once

#include <cstdint>
#include <string_view>

namespace pge
{
    enum OpenGlErrorCode : uint8_t
    {
        OPENGL_ERROR_OK,
        OPENGL_ERROR_GLAD_INIT,
        OPENGL_ERROR_SHADER_CREATION,
    };

    std::string_view opengl_error_message(OpenGlErrorCode code);
}