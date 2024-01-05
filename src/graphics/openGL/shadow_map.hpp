#pragma once

#include "gl_framebuffer.hpp"

namespace pge
{
	struct ShadowMap
	{
		int width = 4096;
		int height = 4096;

		float near = 1.0f;
		float far = 25.0f;

		GlFramebuffer framebuffer;

		uint32_t init_framebuffer();
	};
}