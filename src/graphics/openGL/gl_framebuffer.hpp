#pragma once

#include <glad/glad.h>
#include "../framebuffer_interface.hpp"
#include "../../application/engine.hpp"
#include "../../application/window_interface.hpp"

#define PGE_GL_MAX_FB_TEXTURES 2

namespace pge
{
    struct GlFramebuffer : public IFramebuffer
    {
        GLuint fbo;
        GLuint rbo;
		GLsizei texture_count = 1;
        GLuint textures[PGE_GL_MAX_FB_TEXTURES];
		GLsizei samples = 0;
		GLuint tex_target = GL_TEXTURE_2D;
		GLuint internal_format = GL_RGB16F;
		GLuint pixel_format = GL_RGB;

        ~GlFramebuffer() override;

		void set_samples(int n) override;

        void bind() override;

        void unbind() override;

        uint32_t get_texture() const override;

		Image get_image() const override;

        Connection<void, IWindow*, int, int> *m_on_resize_con;

        void on_resize(IWindow*, int width, int height);

		void set_buffers(int width, int height);
	};

	uint32_t create_color_buffer(GlFramebuffer &fb);
}
