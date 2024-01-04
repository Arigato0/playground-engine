#pragma once

#include "gl_framebuffer.hpp"

namespace pge
{
	struct ShadowMap
	{
		int width = 2048;
		int height = 2048;

		float near = 1.0f;
		float far = 25.0f;

		GlFramebuffer framebuffer;

		uint32_t init_framebuffer();
	};
}