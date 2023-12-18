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

#define WINDOW_PTR (GLFWwindow*)Engine::window.handle()

uint32_t pge::OpenglRenderer::init()
{
    auto result = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (!result)
    {
        return OPENGL_ERROR_GLAD_INIT;
    }

    auto [width, height] = Engine::window.framebuffer_size();

    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(WINDOW_PTR,
        [](GLFWwindow *_, int width, int height)
        {
            glViewport(0, 0, width, height);
        });

    create_texture("assets/missing.jpeg", m_missing_texture, false, TextureWrapMode::Repeat);

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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_STENCIL_TEST);

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



    return OPENGL_ERROR_OK;
}

pge::IShader* pge::OpenglRenderer::create_shader(ShaderList shaders)
{
    // auto id = m_shaders.size();
    //
    // auto &iter = m_shaders.emplace_back();
    //
    // iter.create(shaders);

    return nullptr;
}

void pge::OpenglRenderer::create_buffers(Mesh &mesh)
{
    GlBuffers gl_mesh;

    glGenVertexArrays(1, &gl_mesh.vao);
    glGenBuffers(1, &gl_mesh.vbo);
    glGenBuffers(1, &gl_mesh.ebo);

    glBindVertexArray(gl_mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, gl_mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t),
                 mesh.indices.data(), GL_STATIC_DRAW);

    // vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, coord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    mesh.id = m_buffers.create(gl_mesh);
}

void pge::OpenglRenderer::delete_buffers(Mesh& mesh)
{
    auto buffers = m_buffers.get(mesh.id);

    glDeleteVertexArrays(1, &buffers.vbo);
    glDeleteBuffers(1, &buffers.vao);
    glDeleteBuffers(1, &buffers.ebo);

    m_buffers.remove(mesh.id);
    mesh.id = UINT32_MAX;
}

void pge::OpenglRenderer::set_visualize_depth(bool value)
{
    m_shader.use();
    m_shader.set("visualize_depth", value);
}

void pge::OpenglRenderer::new_frame()
{
    glfwSwapBuffers(WINDOW_PTR);

    glClearColor(EXPAND_VEC4(clear_color));

    glDepthFunc(GL_LESS);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void pge::OpenglRenderer::end_frame()
{
    for (auto &m_sorted_mesh : std::ranges::reverse_view(m_sorted_meshes))
    {
        auto [mesh, model, options] = m_sorted_mesh.second;
        handle_draw(mesh, model, options);
    }

    m_sorted_meshes.clear();
}

void draw_mesh(const pge::MeshView &mesh)
{
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    pge::Engine::statistics.report_draw_call();
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

uint32_t pge::OpenglRenderer::draw(const MeshView &mesh, glm::mat4 model, DrawOptions options)
{
    if (mesh.material.is_transparent)
    {
        float distance = glm::length2(m_camera->position - glm::vec3{model[3]});
        m_sorted_meshes.emplace(distance, DrawData{mesh, model, options});

        return OPENGL_ERROR_OK;
    }

    return handle_draw(mesh, model, options);
}

uint32_t pge::OpenglRenderer::create_texture(std::string_view path, uint32_t &out_texture,
    bool flip, TextureWrapMode wrap_mode)
{
    stbi_set_flip_vertically_on_load(flip);

    int width, height, channels;

    uint8_t *data = stbi_load(path.data(), &width, &height, &channels, 0);

    DEFER([&data]
    {
        stbi_image_free(data);
    });

    if (data == nullptr)
    {
        out_texture = m_missing_texture;
        return OPENGL_ERROR_TEXTURE_LOADING;
    }

    uint32_t id;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

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

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    out_texture = id;

    return OPENGL_ERROR_OK;
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

uint32_t pge::OpenglRenderer::handle_draw(const MeshView&mesh, glm::mat4 model, DrawOptions options)
{
    if (!m_buffers.valid_id(mesh.id))
    {
        return OPENGL_ERROR_MESH_NOT_FOUND;
    }

    if (options.cull_faces)
    {
        glEnable(GL_CULL_FACE);
    }

    auto buffers = m_buffers.get(mesh.id);

    m_shader.use();

    handle_lighting();

    auto material = mesh.material;

    m_shader.set("material.color", material.color);
    m_shader.set("material.shininess", material.shininess);
    m_shader.set("texture_scale", material.diffuse.scale);
    m_shader.set("material.diffuse.enabled", material.diffuse.enabled);
    m_shader.set("material.specular.enabled", material.specular.enabled);
    m_shader.set("material.transparency", material.transparency);
    m_shader.set("recieve_lighting", material.recieve_lighting);
    m_shader.set("material.diffuse.sampler", 0);
    m_shader.set("material.specular.sampler", 1);
    m_shader.set("model", model);
    m_shader.set("projection", m_camera->projection);
    m_shader.set("view", m_camera->view);
    m_shader.set("view_pos", m_camera->position);
    m_shader.set("near", m_camera->near);
    m_shader.set("far", m_camera->far);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse.id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular.id);

    glBindVertexArray(buffers.vao);

    default_stencil();
    draw_mesh(mesh);

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

    glBindVertexArray(0);

    glDisable(GL_CULL_FACE);

    Engine::statistics.report_verticies(mesh.vertices.size());

    return OPENGL_ERROR_OK;
}
