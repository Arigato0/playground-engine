#include "gl_framebuffer.hpp"

#include <glad/glad.h>

#include "opengl_error.hpp"
#include "../../application/engine.hpp"

#define SET_TEX_IMAGE(samples, width, height) (samples) > 0 ? \
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (samples), GL_RGB, (width), (height), GL_TRUE) : \
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (width), (height), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr)

#define SET_RENDER_BUFFER(samples, width, height) (samples) > 0 ? \
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, (width), (height))  : \
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (width), (height))

#define GET_TARGET(samples) (samples) > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D

uint32_t pge::GlFramebuffer::init(int msaa_samples, int internal_format)
{
	auto [width, height] = Engine::window.framebuffer_size();

	samples = msaa_samples;

	tex_target = GET_TARGET(samples);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &texture);
    glBindTexture(tex_target, texture);

	SET_TEX_IMAGE(samples, width, height);

    glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_target, texture, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	SET_RENDER_BUFFER(samples, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return OPENGL_ERROR_FRAMEBUFFER_CREATION;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(tex_target, 0);

    m_on_resize_con = Engine::window.on_framebuffer_resize.connect(this, &GlFramebuffer::on_resize);

    return OPENGL_ERROR_OK;
}

pge::GlFramebuffer::~GlFramebuffer()
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteTextures(1, &texture);

	// TODO uncommenting this causes a segfault investigate why
    //Engine::window.on_framebuffer_resize.disconnect(m_on_resize_con);
}

uint32_t pge::GlFramebuffer::get_texture() const
{
    return texture;
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
    glBindTexture(tex_target, texture);

    Image img;

    glGetTexLevelParameteriv(tex_target, 0, GL_TEXTURE_WIDTH, &img.width);
    glGetTexLevelParameteriv(tex_target, 0, GL_TEXTURE_HEIGHT, &img.height);

    img.data.resize(img.width * img.height * 3);

    glGetTexImage(tex_target, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data.data());

    glBindTexture(tex_target, 0);

    img.channels = 3;

    return img;
}

void pge::GlFramebuffer::on_resize(IWindow*, int width, int height) const
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

void pge::GlFramebuffer::set_buffers(int width, int height) const
{
	glBindTexture(tex_target, texture);

	SET_TEX_IMAGE(samples, width, height);

    glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_target, texture, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	SET_RENDER_BUFFER(samples, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(tex_target, 0);
}

pge::GlFramebuffer pge::create_depth_buffer(int width, int height)
{
	GlFramebuffer buffer;

	glGenTextures(1, &buffer.texture);
	glBindTexture(GL_TEXTURE_2D, buffer.texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.texture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return buffer;
}
