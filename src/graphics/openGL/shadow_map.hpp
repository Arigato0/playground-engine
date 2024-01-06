#pragma once

#include "gl_framebuffer.hpp"

namespace pge
{
	uint32_t create_shadow_map(int width, int height, GlFramebuffer &fb);
}