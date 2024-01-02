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

uint32_t pge::GlFramebuffer::init(int samples)
{
    auto [width, height] = Engine::window.framebuffer_size();

	m_tex_target = samples > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_texture);
    glBindTexture(m_tex_target, m_texture);

	SET_TEX_IMAGE(samples, width, height);

    glTexParameteri(m_tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(m_tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_tex_target, m_texture, 0);

    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);

	SET_RENDER_BUFFER(samples, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return OPENGL_ERROR_FRAMEBUFFER_CREATION;
    }

	m_samples = samples;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(m_tex_target, 0);

    m_on_resize_con = Engine::window.on_framebuffer_resize.connect(this, &GlFramebuffer::on_resize);

    return OPENGL_ERROR_OK;
}

pge::GlFramebuffer::~GlFramebuffer()
{
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteRenderbuffers(1, &m_rbo);
    glDeleteTextures(1, &m_texture);

	// TODO uncommenting this causes a segfault investigate why
    //Engine::window.on_framebuffer_resize.disconnect(m_on_resize_con);
}

uint32_t pge::GlFramebuffer::get_texture() const
{
    return m_texture;
}

void pge::GlFramebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void pge::GlFramebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

pge::Image pge::GlFramebuffer::get_image() const
{
    glBindTexture(m_tex_target, m_texture);

    Image img;

    glGetTexLevelParameteriv(m_tex_target, 0, GL_TEXTURE_WIDTH, &img.width);
    glGetTexLevelParameteriv(m_tex_target, 0, GL_TEXTURE_HEIGHT, &img.height);

    img.data.resize(img.width * img.height * 3);

    glGetTexImage(m_tex_target, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data.data());

    glBindTexture(m_tex_target, 0);

    img.channels = 3;

    return img;
}

void pge::GlFramebuffer::on_resize(IWindow*, int width, int height) const
{
    glBindTexture(m_tex_target, m_texture);

	SET_TEX_IMAGE(m_samples, width, height);

    glTexParameteri(m_tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(m_tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_tex_target, m_texture, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);

	SET_RENDER_BUFFER(m_samples, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
}

void pge::GlFramebuffer::set_samples(int n)
{
 	auto [width, height] = Engine::window.framebuffer_size();

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_texture);

    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, n, GL_RGB, width, height, GL_TRUE);

	m_samples = n;
}