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

    create_texture_from_path("assets/missing.jpeg", m_missing_texture, false, TextureWrapMode::Repeat);

    m_shader.create
   ({
       {PGE_FIND_SHADER("shader.vert"), ShaderType::Vertex},
       {PGE_FIND_SHADER("lighting.frag"), ShaderType::Fragment}
   });

    m_outline_shader.create
   ({
       {PGE_FIND_SHADER("extrude_from_normals.vert"), ShaderType::Vertex},
       {PGE_FIND_SHADER("color.frag"), ShaderType::Fragment}
   });

    m_screen_shader.create
   ({
       {PGE_FIND_SHADER("screen.vert"), ShaderType::Vertex},
       {PGE_FIND_SHADER("screen.frag"), ShaderType::Fragment}
   });

    m_screen_shader.use();
    m_screen_shader.set("screen_texture", 0);

    m_skybox_shader.create
   ({
       {PGE_FIND_SHADER("skybox.vert"), ShaderType::Vertex},
       {PGE_FIND_SHADER("skybox.frag"), ShaderType::Fragment}
   });

    m_skybox_shader.use();
    m_skybox_shader.set("skybox_texture", 0);

    create_screen_plane();
    create_skybox_cube();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    VALIDATE_ERR(m_out_buffer.init());

    return m_screen_buffer.init();
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
    m_shader.use();
    m_shader.set("visualize_depth", value);
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

uint32_t pge::OpenglRenderer::create_texture_from_path(std::string_view path, uint32_t& out_texture, bool flip,
    TextureWrapMode wrap_mode)
{
    stbi_set_flip_vertically_on_load(flip);

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

    return create_texture(data, width, height, channels, out_texture, wrap_mode);
}

uint32_t pge::OpenglRenderer::create_texture(ustring_view data, int width, int height, int channels,
    uint32_t &out_texture, TextureWrapMode wrap_mode)
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

    int format;

    if (channels == 1)
    {
        format = GL_RED;
    }
    if (channels == 3)
    {
        format = GL_RGB;
    }
    if (channels == 4)
    {
        format = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    return OPENGL_ERROR_OK;
}

uint32_t pge::OpenglRenderer::create_cubemap_from_path(std::array<std::string_view, 6> faces, uint32_t& out_texture)
{
    glGenTextures(1, &out_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, out_texture);

    int width, height, channels;

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
    m_shader.set("light_count", (int)Light::table.size());

    auto light_iter = Light::table.begin();

    for (int i = 0; i < Light::table.size(); i++)
    {
        auto *light = *light_iter++;

        if (light == nullptr)
        {
            continue;
        }

        static char name_buffer[256];

        auto start = sprintf(name_buffer, "lights[%i].", i);

        static auto field = [start](std::string_view name) -> const char*
        {
            sprintf(name_buffer + start, "%s", name.data());
            return name_buffer;
        };

        m_shader.set(field("is_active"), light->is_active);
        m_shader.set(field("color"), light->color);
        m_shader.set(field("diffuse"), light->diffuse);
        m_shader.set(field("specular"), light->specular);
        m_shader.set(field("ambient"), light->ambient);
        m_shader.set(field("power"), light->power);
        m_shader.set(field("direction"), m_camera->front);
        m_shader.set(field("cutoff"), light->inner_cutoff);
        m_shader.set(field("outer_cutoff"), light->outer_cutoff);
        m_shader.set(field("constant"),  light->constant);
        m_shader.set(field("linear"),    light->linear);
        m_shader.set(field("quadratic"), light->quadratic);
        m_shader.set(field("is_spot"), light->is_spot);

        if (light->position)
        {
            m_shader.set(field("position"), *light->position);
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

    if (!options.cull_faces)
    {
        glDisable(GL_CULL_FACE);
    }

    auto buffers = m_buffers.get(mesh.id);

    glBindVertexArray(buffers.vao);

    draw_mesh(mesh);

    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);

    return OPENGL_ERROR_OK;
}

void pge::OpenglRenderer::draw_passes()
{
    m_screen_buffer.bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    draw_skybox();

    draw_everything();

    m_screen_buffer.unbind();

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

void pge::OpenglRenderer::draw_everything()
{
    glEnable(GL_DEPTH_TEST);

    for (auto &data : m_render_queue)
    {
        set_base_uniforms(data);
        handle_draw(data);
    }

    for (auto &[_, data] : std::ranges::reverse_view(m_sorted_meshes))
    {
        set_base_uniforms(data);
        handle_draw(data);
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

    m_shader.use();

    handle_lighting();

    auto material = mesh.material;

    m_shader.set("material.color", material.color);
    m_shader.set("material.shininess", material.shininess);
    m_shader.set("texture_scale", material.diffuse.scale);
    m_shader.set("material.diffuse.enabled", material.diffuse.enabled);
    m_shader.set("material.specular.enabled", material.specular.enabled);
    m_shader.set("material.transparency", material.alpha);
    m_shader.set("recieve_lighting", material.recieve_lighting);
    m_shader.set("material.diffuse.sampler", 0);
    m_shader.set("material.specular.sampler", 1);

    m_shader.set("model", data.model);
    m_shader.set("projection", m_camera->projection);
    m_shader.set("view", m_camera->view);
    m_shader.set("view_pos", m_camera->position);
    m_shader.set("near", m_camera->near);
    m_shader.set("far", m_camera->far);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse.id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular.id);
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

    glDepthMask(GL_FALSE);

    m_skybox_shader.use();

    auto view = glm::mat4(glm::mat3(m_camera->view));
    m_skybox_shader.set("projection", m_camera->projection);
    m_skybox_shader.set("view", view);

    glBindVertexArray(m_skybox_cube.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox_texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthMask(GL_TRUE);
}
