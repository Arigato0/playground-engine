#pragma once

#include "gl_framebuffer.hpp"

namespace pge
{
	struct ShadowMap
	{
		int width = 2048;
		int height = 2048;
		GlFramebuffer framebuffer;

		uint32_t init_framebuffer();
	};
}