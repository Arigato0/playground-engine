#include "gl_framebuffer.hpp"

#include <glad/glad.h>

#include "opengl_error.hpp"
#include "../../application/engine.hpp"

#define SET_TEX_IMAGE(fb, width, height) (fb.samples) > 0 ? \
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (fb.samples), (fb.internal_format), (width), (height), GL_TRUE) : \
	glTexImage2D(GL_TEXTURE_2D, 0, (fb.internal_format), (width), (height), 0, (fb.pixel_format), GL_UNSIGNED_BYTE, nullptr)

#define SET_RENDER_BUFFER(fb, width, height) (fb.samples) > 0 ? \
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, (fb.samples), GL_DEPTH24_STENCIL8, (width), (height))  : \
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (width), (height))

#define GET_TARGET(samples) (samples) > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D

pge::GlFramebuffer::~GlFramebuffer()
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteTextures(texture_count, textures);

	if (m_on_resize_con)
	{
    	Engine::window.on_framebuffer_resize.disconnect(m_on_resize_con);
	}
}

uint32_t pge::GlFramebuffer::get_texture() const
{
    return textures[0];
}

void pge::GlFramebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void pge::GlFramebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

pge::Image pge::GlFramebuffer::get_image() const
{
    glBindTexture(tex_target, textures[0]);

    Image img;

    glGetTexLevelParameteriv(tex_target, 0, GL_TEXTURE_WIDTH, &img.width);
    glGetTexLevelParameteriv(tex_target, 0, GL_TEXTURE_HEIGHT, &img.height);

    img.data.resize(img.width * img.height * 3);

    glGetTexImage(tex_target, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data.data());

    glBindTexture(tex_target, 0);

    img.channels = 3;

    return img;
}

void pge::GlFramebuffer::on_resize(IWindow*, int width, int height)
{
	set_buffers(width, height);
}

void pge::GlFramebuffer::set_samples(int n)
{
 	auto [width, height] = Engine::window.framebuffer_size();

	tex_target = GET_TARGET(n);

	set_buffers(width, height);

	samples = n;
}

void set_textures(const pge::GlFramebuffer &fb, int width, int height)
{
	for (int i = 0; i < fb.texture_count; i++)
	{
		glBindTexture(fb.tex_target, fb.textures[i]);

		SET_TEX_IMAGE(fb, width, height);

		glTexParameteri(fb.tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(fb.tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(fb.tex_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(fb.tex_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, fb.tex_target, fb.textures[i], 0);
	}
}

void pge::GlFramebuffer::set_buffers(int width, int height)
{
	auto &fb = *this;

	set_textures(fb, width, height);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_target, textures[0], 0);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	SET_RENDER_BUFFER(fb, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(tex_target, 0);
}

void pge::GlFramebuffer::blit(pge::IFramebuffer *src, int width, int height, int attachment, int x, int y)
{
	auto gl_src = (GlFramebuffer*)src;

	if (width == 0 && height == 0)
	{
		auto wh = Engine::window.framebuffer_size();
		width = wh.first;
		height = wh.second;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_src->fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + attachment);

	glBlitFramebuffer(x, y, width, height, x, y, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void pge::GlFramebuffer::blit_all_targets(pge::IFramebuffer *src, int width, int height, int x, int y)
{
	auto gl_src = (GlFramebuffer*)src;

	for (int i = 0; i < texture_count && i < gl_src->texture_count; i++)
	{
		blit(src, width, height, i);
	}
}

uint32_t pge::create_color_buffer(GlFramebuffer &fb)
{
	auto [width, height] = Engine::window.framebuffer_size();

    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

	fb.tex_target = GET_TARGET(fb.samples);

    glGenTextures(fb.texture_count, fb.textures);

	set_textures(fb, width, height);

    glGenRenderbuffers(1, &fb.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb.rbo);

	SET_RENDER_BUFFER(fb, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.rbo);

	if (fb.texture_count > 1)
	{
		GLuint attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, attachments);
	}
//	GLuint attachment[PGE_GL_MAX_FB_TEXTURES];
//
//	for (int i = 0; i < fb.texture_count && i < PGE_GL_MAX_FB_TEXTURES; i++)
//	{
//		attachment[i] = GL_COLOR_ATTACHMENT0 + i;
//	}
//
//	glDrawBuffers(fb.texture_count, attachment);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return OPENGL_ERROR_FRAMEBUFFER_CREATION;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(fb.tex_target, 0);

	fb.m_on_resize_con = Engine::window.on_framebuffer_resize.connect(&fb, &GlFramebuffer::on_resize);

	if (fb.m_on_resize_con == nullptr)
	{
		Logger::info("is null");
	}

    return OPENGL_ERROR_OK;
}
