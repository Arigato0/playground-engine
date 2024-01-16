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

	struct RenderStats
	{
		uint32_t vertices = 0;
		uint32_t draw_calls = 0;
	};

	struct TextureSettings
	{
		float anisotropic_level = 4;
		float anisotropic_distance = 100;
	};

    struct ShadowSettings
    {
		int pcf_samples = 20;
		float bias = 0.05;
		bool enable_soft = true;
		int width = 4096;
		int height = 4096;
		float distance = 100.0f;
    };

	struct ScreenSpaceSettings
	{
		float gamma = 1.4;
		float exposure = 2.7;
		float bright_threshold = 1.4;
		bool enable_bloom = true;
		int bloom_blur_passes = 10;
	};

	struct AllRenderSettings
	{
		TextureSettings texture;
		ShadowSettings shadow;
		ScreenSpaceSettings screen_space;
	};
}
