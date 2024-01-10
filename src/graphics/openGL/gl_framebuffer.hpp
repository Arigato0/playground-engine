#pragma once

#include <glad/glad.h>
#include "../framebuffer_interface.hpp"
#include "../../application/engine.hpp"
#include "../../application/window_interface.hpp"

namespace pge
{
    struct GlFramebuffer : public IFramebuffer
    {
        uint32_t fbo;
        uint32_t rbo;
        uint32_t texture;
		int samples;
		int tex_target;
		int internal_format;

        uint32_t init(int msaa_samples = 0, int format = GL_RGB);

        ~GlFramebuffer() override;

		void set_samples(int n) override;

        void bind() override;

        void unbind() override;

        uint32_t get_texture() const override;

		Image get_image() const override;

        Connection<void, IWindow*, int, int> *m_on_resize_con;

        void on_resize(IWindow*, int width, int height) const;

		void set_buffers(int width, int height) const;
	};

	pge::GlFramebuffer create_depth_buffer(int width, int height);
}
