#include "gaussian_blur.hpp"
#include "../../application/engine.hpp"

pge::GaussianBlur::~GaussianBlur()
{
	glDeleteFramebuffers(2, fbos);
	glDeleteTextures(2, textures);
}

uint32_t pge::GaussianBlur::init()
{
	VALIDATE_ERR(shader.create(
	{
		{PGE_FIND_SHADER("quad.vert.glsl"), ShaderType::Vertex},
		{PGE_FIND_SHADER("gaussian_blur.frag.glsl"), ShaderType::Fragment},
	}));

	shader.use();
	shader.set("image", 0);

	glGenFramebuffers(2, fbos);
	glGenTextures(2, textures);

	auto [width, height] = Engine::window.framebuffer_size();

	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
		glBindTexture(GL_TEXTURE_2D, textures[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);

		 if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			return OPENGL_ERROR_FRAMEBUFFER_CREATION;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Engine::window.on_framebuffer_resize.connect(this, &GaussianBlur::on_resize);

	return OPENGL_ERROR_OK;
}

void pge::GaussianBlur::on_resize(pge::IWindow *, int width, int height)
{
	for (auto texture : textures)
	{
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	}
}



