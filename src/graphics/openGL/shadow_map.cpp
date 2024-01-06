#pragma once

#include "shadow_map.hpp"
#include "opengl_error.hpp"
#include <glad/glad.h>

uint32_t pge::create_shadow_map(int width, int height, GlFramebuffer &fb)
{
	fb.tex_target = GL_TEXTURE_CUBE_MAP;

	glGenTextures(1, &fb.texture);
	glBindTexture(fb.tex_target, fb.texture);

	for (int i = 0; i < 6; ++i)
	{
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	glTexParameteri(fb.tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(fb.tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

 	glTexParameteri(fb.tex_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(fb.tex_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(fb.tex_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &fb.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fb.texture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(fb.tex_target, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return OPENGL_ERROR_FRAMEBUFFER_CREATION;
    }

	return OPENGL_ERROR_OK;
}

