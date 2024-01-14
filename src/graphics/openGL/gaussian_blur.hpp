#pragma once

#include "opengl_shader.hpp"
#include "../../application/window_interface.hpp"

namespace pge
{
	struct GaussianBlur
	{
		~GaussianBlur();

		uint32_t init();

		GlShader shader;
		GLuint fbos[2];
		GLuint textures[2];
		bool horizontal = true;

	private:
		void on_resize(IWindow*, int width, int height);
	};
}