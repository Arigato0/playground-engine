#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace pge
{
    enum class OutlineMethod : uint8_t
    {
        // uses the stencil buffer to create outlines
        Stencil,
    };

    struct OutlineOptions
    {
        glm::vec4 color;
        float line_thickness = 0.05;
        bool depth_test = false;
        OutlineMethod method;
    };

    struct DrawOptions
    {
        bool enable_outline = false;
        OutlineOptions outline;
        bool cull_faces = true;
    };

    struct RendererSettings
    {

    };
}
