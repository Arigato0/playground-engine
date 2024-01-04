#include "opengl_renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include "opengl_error.hpp"

#include "../../application/engine.hpp"
#include "../light.hpp"

#include <glm/gtx/norm.hpp>
#include <ranges>

#include "../primitives.hpp"
#include "../../data/string.hpp"

uint32_t pge::OpenglRenderer::init()
{
    auto result = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (!result)
    {
        return OPENGL_ERROR_GLAD_INIT;
    }

    Engine::window.on_framebuffer_resize.connect(
    [](IWindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
    });

	create_texture_from_path("assets/missing.jpeg", m_missing_texture, TextureOptions{});

	using enum ShaderType;

    m_lighting_shader.create
   ({
       {PGE_FIND_SHADER("lighting.vert"), Vertex},
       {PGE_FIND_SHADER("lighting.frag"), Fragment}
   });

    m_outline_shader.create
   ({
       {PGE_FIND_SHADER("extrude_from_normals.vert"), Vertex},
       {PGE_FIND_SHADER("color.frag"), Fragment}
   });

    m_screen_shader.create
   ({
       {PGE_FIND_SHADER("screen.vert"), Vertex},
       {PGE_FIND_SHADER("screen.frag"), Fragment}
   });

    m_screen_shader.use();
    m_screen_shader.set("screen_texture", 0);
	m_screen_shader.set("gamma", 1.7f);

	m_shadow_map_shader.create
   ({
       {PGE_FIND_SHADER("shadow_map.vert"), Vertex},
       {PGE_FIND_SHADER("omni_shadow_map.geo.glsl"), Geometry},
       {PGE_FIND_SHADER("shadow_map.frag.glsl"), Fragment},
   });

    m_skybox_shader.create
   ({
       {PGE_FIND_SHADER("skybox.vert"), Vertex},
       {PGE_FIND_SHADER("skybox.frag"), Fragment}
   });

    m_skybox_shader.use();
    m_skybox_shader.set("skybox_texture", 0);

    create_screen_plane();
    create_skybox_cube();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    //glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	VALIDATE_ERR(m_render_buffer.init(4));

	VALIDATE_ERR(m_out_buffer.init());
	VALIDATE_ERR(m_screen_buffer.init());

	VALIDATE_ERR(m_shadow_map.init_framebuffer());

    return OPENGL_ERROR_OK;
}

pge::IShader* pge::OpenglRenderer::create_shader(ShaderList shaders)
{
    auto &iter = m_shaders.emplace_back();

    iter.create(shaders);

    return &iter;
}

void pge::OpenglRenderer::create_buffers(Mesh &mesh)
{
    auto buffer = GlBufferBuilder()
        .start()
        .stride(sizeof(Vertex))
        .vbo(mesh.vertices)
        .ebo(mesh.indices)
        .attr(3, offsetof(Vertex, position))
        .attr(3, offsetof(Vertex, normal))
        .attr(2, offsetof(Vertex, coord))
        .finish();

    mesh.id = m_buffers.create(buffer);
}

void pge::OpenglRenderer::delete_buffers(Mesh& mesh)
{
    m_delete_queue.push_back(mesh.id);
}

void pge::OpenglRenderer::set_visualize_depth(bool value)
{
    m_lighting_shader.use();
    m_lighting_shader.set("visualize_depth", value);
}

void disable_stencil()
{
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
}

void enable_stencil()
{
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
}

void default_stencil()
{
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
}

void pge::OpenglRenderer::new_frame()
{
}

void pge::OpenglRenderer::end_frame()
{
    draw_passes();
    handle_gl_buffer_delete();
    clear_buffers();
}

void draw_mesh(const pge::MeshView &mesh)
{
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
    pge::Engine::statistics.report_draw_call();
}

void pge::OpenglRenderer::draw(const MeshView&mesh, glm::mat4 model, DrawOptions options)
{
    DrawData data {mesh, model, options};

    if (mesh.material.use_alpha)
    {
        float distance = glm::length2(m_camera->position - glm::vec3{model[3]});
        m_sorted_meshes.emplace(distance, std::move(data));
    }
    else
    {
        m_render_queue.emplace_back(std::move(data));
    }
}

uint32_t
pge::OpenglRenderer::create_texture_from_path(std::string_view path, uint32_t &out_texture, TextureOptions options)
{
    stbi_set_flip_vertically_on_load(options.flip);

    int width, height, channels;

    auto *data = stbi_load(path.data(), &width, &height, &channels, 0);

    if (data == nullptr)
    {
        out_texture = m_missing_texture;
        return OPENGL_ERROR_TEXTURE_LOADING;
    }

    DEFER([&data]
    {
        stbi_image_free(data);
    });

    return create_texture(data, width, height, channels, out_texture, options.wrap_mode, options.gamma_correct);
}

uint32_t
pge::OpenglRenderer::create_texture(ustring_view data, int width, int height, int channels, uint32_t &out_texture,
	TextureWrapMode wrap_mode,
	bool gamma_correct)
{
    glGenTextures(1, &out_texture);
    glBindTexture(GL_TEXTURE_2D, out_texture);

    int wrap;
    using enum TextureWrapMode;

    switch (wrap_mode)
    {
        case Repeat: wrap = GL_REPEAT; break;
        case MirroredRepeat: wrap = GL_MIRRORED_REPEAT; break;
        case ClampToEdge: wrap = GL_CLAMP_TO_EDGE; break;
        case ClampToBorder: wrap = GL_CLAMP_TO_BORDER; break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int pixel_format;
	int internal_format;

    if (channels == 1)
    {
		pixel_format = GL_RED;
		internal_format = GL_RED;
    }
    if (channels == 3)
    {
		pixel_format = GL_RGB;
		internal_format = GL_SRGB;
    }
    if (channels == 4)
    {
		pixel_format = GL_RGBA;
		internal_format = GL_SRGB_ALPHA;
    }

	internal_format = gamma_correct ? internal_format : pixel_format;

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    return OPENGL_ERROR_OK;
}

uint32_t pge::OpenglRenderer::create_cubemap_from_path(std::array<std::string_view, 6> faces, uint32_t& out_texture)
{
    glGenTextures(1, &out_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, out_texture);

    int width, height, channels;

	stbi_set_flip_vertically_on_load(false);

    for (int i = 0; i < faces.size(); i++)
    {
        auto *data = stbi_load(faces[i].data(), &width, &height, &channels, 0);

        if (!data)
        {
            out_texture = m_missing_texture;
            return OPENGL_ERROR_TEXTURE_LOADING;
        }

        DEFER([&data]
        {
            stbi_image_free(data);
        });

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return OPENGL_ERROR_OK;
}

pge::Image pge::OpenglRenderer::get_image()
{
    Image img;

    auto [width, height] = Engine::window.framebuffer_size();

    img.width = width;
    img.height = height;

    img.data.resize(img.width * img.height * 3);

    img.channels = 3;

    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, img.data.data());

    return img;
}

void pge::OpenglRenderer::delete_texture(uint32_t id)
{
    if (id == m_missing_texture)
    {
        return;
    }

    glDeleteTextures(1, &id);
}

pge::RendererProperties pge::OpenglRenderer::properties()
{
    RendererProperties output
    {
        .device_name = (const char*)glGetString(GL_RENDERER),
        .api = GraphicsApi::OpenGl
    };

    glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&output.version_major);
    glGetIntegerv(GL_MINOR_VERSION, (GLint*)&output.version_minor);

    return output;
}

void pge::OpenglRenderer::draw_shaded_wireframe(const Mesh& mesh, glm::mat4 model)
{
    m_outline_shader.use();

    m_outline_shader.set("projection", m_camera->projection);
    m_outline_shader.set("view", m_camera->view);
    m_outline_shader.set("model", model);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glPolygonOffset( -1.f, 0);

    draw_mesh(mesh);

    glDisable( GL_POLYGON_OFFSET_FILL );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void pge::OpenglRenderer::handle_lighting()
{
	m_lighting_shader.use();

    m_lighting_shader.set("light_count", (int)Light::table.size());

    auto light_iter = Light::table.begin();

	char name_buffer[256];

    for (int i = 0; i < Light::table.size(); i++)
    {
        auto *light = *light_iter++;

        if (light == nullptr)
        {
            continue;
        }

        auto start = sprintf(name_buffer, "lights[%i].", i);

        static auto field = [&name_buffer, start](std::string_view name) -> const char*
        {
            sprintf(name_buffer + start, "%s", name.data());
            return name_buffer;
        };

        m_lighting_shader.set(field("is_active"), light->is_active);
        m_lighting_shader.set(field("color"), light->color);
        m_lighting_shader.set(field("diffuse"), light->diffuse);
        m_lighting_shader.set(field("specular"), light->specular);
        m_lighting_shader.set(field("ambient"), light->ambient);
        m_lighting_shader.set(field("power"), light->power);
        m_lighting_shader.set(field("direction"), m_camera->front);
        m_lighting_shader.set(field("cutoff"), light->inner_cutoff);
        m_lighting_shader.set(field("outer_cutoff"), light->outer_cutoff);
        m_lighting_shader.set(field("constant"),  light->constant);
        m_lighting_shader.set(field("linear"),    light->linear);
        m_lighting_shader.set(field("quadratic"), light->quadratic);
        m_lighting_shader.set(field("is_spot"), light->is_spot);

        if (light->position)
        {
            m_lighting_shader.set(field("position"), *light->position);
        }
    }
}

uint32_t pge::OpenglRenderer::handle_draw(const DrawData &data)
{
    auto &[mesh, model, options] = data;

    if (!m_buffers.valid_id(mesh.id))
    {
        return OPENGL_ERROR_MESH_NOT_FOUND;
    }

//    if (!options.cull_faces)
//    {
//        glDisable(GL_CULL_FACE);
//    }

    auto buffers = m_buffers.get(mesh.id);

    glBindVertexArray(buffers.vao);

    draw_mesh(mesh);

    glBindVertexArray(0);

    //glEnable(GL_CULL_FACE);

    return OPENGL_ERROR_OK;
}

void pge::OpenglRenderer::draw_passes()
{
	render_to_framebuffer(m_render_buffer);

	auto *main_camera = m_camera;

    for (auto &view : m_render_views)
    {
		if (!view.is_active || view.framebuffer == nullptr)
		{
			continue;
		}

		m_camera = view.camera;

		render_to_framebuffer(*((GlFramebuffer*)view.framebuffer));
    }

	m_camera = main_camera;

    if (m_is_offline)
    {
        m_out_buffer.bind();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    draw_screen_plane();

	if (m_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
	}

    m_out_buffer.unbind();
}

void pge::OpenglRenderer::draw_everything(bool calculate_shadows)
{
    glEnable(GL_DEPTH_TEST);

	auto draw_data = [&]
	(DrawData &data)
	{
		if (calculate_shadows)
		{
			m_shadow_map_shader.use();
			m_shadow_map_shader.set("model", data.model);
		}
		else
		{
			set_base_uniforms(data);
		}

        handle_draw(data);
	};

    for (auto &data : m_render_queue)
    {
		draw_data(data);
    }

    for (auto &[_, data] : std::ranges::reverse_view(m_sorted_meshes))
    {
		draw_data(data);
    }
}

void pge::OpenglRenderer::clear_buffers()
{
    m_render_queue.clear();
    m_sorted_meshes.clear();
    m_delete_queue.clear();
}

void pge::OpenglRenderer::draw_outline(const DrawData& data)
{
    auto &[mesh, model, options] = data;

    if (options.enable_outline)
    {
        enable_stencil();

        if (!options.outline.depth_test)
        {
            glDisable(GL_DEPTH_TEST);
        }

        m_outline_shader.use();

        m_outline_shader.set("projection", m_camera->projection);
        m_outline_shader.set("view", m_camera->view);
        m_outline_shader.set("model", model);
        m_outline_shader.set("color", options.outline.color);
        m_outline_shader.set("extrude_mul", options.outline.line_thickness);

        draw_mesh(mesh);

        disable_stencil();

        glEnable(GL_DEPTH_TEST);

        glClear(GL_STENCIL_BUFFER_BIT);
    }
}

void pge::OpenglRenderer::handle_gl_buffer_delete()
{
    for (auto id : m_delete_queue)
    {
        auto buffers = m_buffers.get(id);

        buffers.free();

        m_buffers.remove(id);
    }
}

void pge::OpenglRenderer::set_base_uniforms(const DrawData &data)
{
    auto &[mesh, model, _] = data;

    m_lighting_shader.use();

    auto material = mesh.material;

    m_lighting_shader.set("material.color", material.color);
    m_lighting_shader.set("material.shininess", material.shininess);
    m_lighting_shader.set("texture_scale", material.diffuse.scale);
    m_lighting_shader.set("material.diffuse.enabled", material.diffuse.enabled);
    m_lighting_shader.set("material.transparency", material.alpha);
    m_lighting_shader.set("recieve_lighting", material.recieve_lighting);
    m_lighting_shader.set("material.diffuse.sampler", 0);
    m_lighting_shader.set("material.specular", material.specular);

    m_lighting_shader.set("model", data.model);
    m_lighting_shader.set("projection", m_camera->projection);
    m_lighting_shader.set("view", m_camera->view);
    m_lighting_shader.set("view_pos", m_camera->position);
    m_lighting_shader.set("camera_near", m_camera->near);
    m_lighting_shader.set("camera_far", m_camera->far);
	m_lighting_shader.set("shadow_far", m_shadow_map.far);

	m_lighting_shader.set("shadow_map", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse.id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(m_shadow_map.framebuffer.tex_target, m_shadow_map.framebuffer.texture);
}

void pge::OpenglRenderer::create_screen_plane()
{
    m_screen_plane = GlBufferBuilder()
        .start()
        .vbo(QUAD_MESH)
        .stride(4 * sizeof(float))
        .attr(2, 0)
        .attr(2, 2 * sizeof(float))
        .finish();
}

void pge::OpenglRenderer::create_skybox_cube()
{
    m_skybox_cube = GlBufferBuilder()
        .start()
        .vbo(CUBE_VERTS)
        .stride(3 * sizeof(float))
        .attr(3, 0)
        .finish();
}

void pge::OpenglRenderer::draw_screen_plane()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    m_screen_shader.use();

    auto [width, height] = Engine::window.framebuffer_size();

    m_screen_shader.set("resolution", {width, height});

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_screen_buffer.get_texture());

    glBindVertexArray(m_screen_plane.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void pge::OpenglRenderer::draw_skybox()
{
    if (m_skybox_texture == UINT32_MAX)
    {
        return;
    }

    m_skybox_shader.use();

    auto view = glm::mat4(glm::mat3(m_camera->view));
    m_skybox_shader.set("projection", m_camera->projection);
    m_skybox_shader.set("view", view);

    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(m_skybox_cube.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox_texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

void pge::OpenglRenderer::render_to_framebuffer(pge::GlFramebuffer &fb)
{
	render_to_shadow_map();

	fb.bind();

	auto [width, height] = Engine::window.framebuffer_size();

	glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    handle_lighting();
	draw_everything(false);
    draw_skybox();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_screen_buffer.fbo);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    fb.unbind();
}

pge::RenderView *pge::OpenglRenderer::add_view(pge::Camera *camera)
{
	auto *fb = new GlFramebuffer();

	auto result = fb->init();

	if (result != 0)
	{
		delete fb;
		return nullptr;
	}

	auto &view = m_render_views.emplace_back(camera, fb, true);

	view.camera = camera;
	view.iter = --m_render_views.end();

	return &m_render_views.back();
}

void pge::OpenglRenderer::remove_view(RenderView *view)
{
	if (view == nullptr)
	{
		return;
	}

	delete view->framebuffer;

	view->framebuffer = nullptr;

	m_render_views.erase(view->iter);
}

void pge::OpenglRenderer::render_to_shadow_map()
{
	glViewport(0, 0, m_shadow_map.width, m_shadow_map.height);

	m_shadow_map.framebuffer.bind();

	m_shadow_map_shader.use();

	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	auto projection = glm::perspective(glm::radians(90.0f), float(m_shadow_map.width / m_shadow_map.height),
		m_shadow_map.near, m_shadow_map.far);

	auto *light = Light::table.front();
	auto light_pos = *light->position;

	std::array shadow_transforms =
	{
		 projection * glm::lookAt(light_pos, light_pos + glm::vec3{ 1.0, 0.0, 0.0}, glm::vec3{0.0,-1.0, 0.0}),
		 projection * glm::lookAt(light_pos, light_pos + glm::vec3{-1.0, 0.0, 0.0}, glm::vec3{0.0,-1.0, 0.0}),
		 projection * glm::lookAt(light_pos, light_pos + glm::vec3{0.0, 1.0, 0.0}, glm::vec3{0.0, 0.0, 1.0}),
		 projection * glm::lookAt(light_pos, light_pos + glm::vec3{0.0,-1.0, 0.0}, glm::vec3{0.0, 0.0,-1.0}),
		 projection * glm::lookAt(light_pos, light_pos + glm::vec3{0.0, 0.0, 1.0}, glm::vec3{0.0,-1.0, 0.0}),
		 projection * glm::lookAt(light_pos, light_pos + glm::vec3{0.0, 0.0,-1.0}, glm::vec3{0.0,-1.0, 0.0}),
	};

	m_shadow_map_shader.set("far_plane", m_shadow_map.far);
	m_shadow_map_shader.set("light_pos", light_pos);

	for (int i = 0; i < shadow_transforms.size(); ++i)
	{
		m_shadow_map_shader.set(fmt::format("shadow_transforms[{}]", i), shadow_transforms[i]);
	}

	draw_everything(true);

	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	m_shadow_map.framebuffer.unbind();
}
