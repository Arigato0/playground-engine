#pragma once

#include "shadow_map.hpp"
#include "opengl_error.hpp"
#include <glad/glad.h>

uint32_t pge::ShadowMap::init_framebuffer()
{
	framebuffer.tex_target = GL_TEXTURE_CUBE_MAP;

	glGenTextures(1, &framebuffer.texture);
	glBindTexture(framebuffer.tex_target, framebuffer.texture);

	for (int i = 0; i < 6; ++i)
	{
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	glTexParameteri(framebuffer.tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(framebuffer.tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

 	glTexParameteri(framebuffer.tex_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(framebuffer.tex_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(framebuffer.tex_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebuffer.texture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(framebuffer.tex_target, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return OPENGL_ERROR_FRAMEBUFFER_CREATION;
    }

	return OPENGL_ERROR_OK;
}

