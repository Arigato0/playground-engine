#pragma once

#include <list>
#include "Camera.hpp"
#include "framebuffer_interface.hpp"

namespace pge
{
	struct RenderView;

	using RenderViewList = std::list<RenderView>;

	struct RenderView
	{
		Camera *camera 			  = nullptr;
		IFramebuffer *framebuffer = nullptr;
		bool is_active 			  = true;
		RenderViewList::iterator iter;
	};
}