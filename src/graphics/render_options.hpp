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
        bool cull_faces = false;
    };

    // all the data needed to draw a mesh
    struct DrawData
    {
        const MeshView &mesh;
        const glm::mat4 model;
        const DrawOptions options;
    };

    enum class TextureWrapMode : uint8_t
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder,
    };

	// texture options for loading textures with the renderer
	struct TextureOptions
	{
		bool flip = true;
		bool gamma_correct = true;
		TextureWrapMode wrap_mode = TextureWrapMode::Repeat;
	};

    enum class WireframeMode : uint8_t
    {
        Lines,
        Shaded,
    };

    struct ShadowSettings
    {
		int pcf_samples = 16;
		float bias = 0.15;
		bool enable_soft = true;
		int width = 2048;
		int height = 2048;
    };

	struct RenderColorSettings
	{
		float gamma = 1.4;
	};
}
