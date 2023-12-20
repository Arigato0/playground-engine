#pragma once

#include <cstdint>
#include <string>

namespace pge
{
    enum class GraphicsApi : uint8_t
    {
        OpenGl,
        Vulkan,
    };

    static bool is_implemented(GraphicsApi api)
    {
        using enum GraphicsApi;

        switch (api)
        {
            case Vulkan: return false;
            case OpenGl: return true;
            default: return false;
        }
    }

    static std::string to_string(GraphicsApi api)
    {
        using enum GraphicsApi;

        switch (api)
        {
            case Vulkan: return "Vulkan";
            case OpenGl: return "OpenGL";
            default: return "Unknown";
        }
    }
}
