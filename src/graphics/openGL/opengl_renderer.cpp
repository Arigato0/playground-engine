#include "opengl_renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include "opengl_error.hpp"

#include "../../application/engine.hpp"

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

    create_texture("assets/missing.jpeg", m_missing_texture);

    m_shader.create
   ({
       {PGE_FIND_SHADER("shader.vert"), ShaderType::Vertex},
       {PGE_FIND_SHADER("lighting.frag"), ShaderType::Fragment}
   });

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

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

size_t pge::OpenglRenderer::create_mesh(const Mesh &mesh)
{
    GlMesh gl_mesh;

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

    gl_mesh.data = &mesh;

    auto idx = m_meshes.size();

    m_meshes.emplace_back(gl_mesh);

    return idx;
}

void pge::OpenglRenderer::new_frame()
{
    glfwSwapBuffers(WINDOW_PTR);

    glClearColor(EXPAND_VEC4(clear_color));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

uint32_t pge::OpenglRenderer::draw(size_t mesh_id, glm::mat4 transform)
{
    if (mesh_id >= m_meshes.size())
    {
        return OPENGL_ERROR_MESH_NOT_FOUND;
    }

    auto mesh = m_meshes[mesh_id];

    if (mesh.data == nullptr)
    {
        return OPENGL_ERROR_MESH_NOT_FOUND;
    }

    m_shader.use();

    m_shader.set("light_count", (int)Light::table.size());

    auto material = mesh.data->material;

    m_shader.set("object_color", material.color);
    m_shader.set("material.shininess", material.shininess);
    m_shader.set("texture_scale", material.diffuse.scale);
    m_shader.set("material.diffuse.enabled", material.diffuse.enabled);
    m_shader.set("material.specular.enabled", material.specular.enabled);
    m_shader.set("recieve_lighting", material.recieve_lighting);

    for (int i = 0; i < Light::table.size(); i++)
    {
        auto *light = Light::table[i];

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

    m_shader.set("material.diffuse.sampler", 0);
    m_shader.set("material.specular.sampler", 1);
    m_shader.set("model", transform);
    m_shader.set("projection", m_camera->projection);
    m_shader.set("view", m_camera->view);
    m_shader.set("view_pos", m_camera->position);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.diffuse.id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.specular.id);

    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.data->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    Engine::statistics.report_draw_call();
    Engine::statistics.report_verticies(mesh.data->vertices.size());

    return OPENGL_ERROR_OK;
}

uint32_t pge::OpenglRenderer::create_texture(std::string_view path, uint32_t &out_texture)
{
    stbi_set_flip_vertically_on_load(false);
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto format = channels > 3 ? GL_RGBA : GL_RGB;

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
