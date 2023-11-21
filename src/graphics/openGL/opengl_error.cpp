#pragma once

#include <array>
#include "opengl_error.hpp"

constexpr std::array ERROR_MESSAGES
{
    "No error",
    "Could not load the glad library",
    "Could not create shader",
    "Could not load or generate texture",
    "could not find the specified mesh"
};

std::string_view pge::opengl_error_message(OpenGlErrorCode code)
{
    auto idx = (int)code;

    if (idx >= ERROR_MESSAGES.size())
    {
        return {};
    }

    return ERROR_MESSAGES[idx];
}