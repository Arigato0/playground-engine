#include "opengl_renderer.hpp"

//#define STB_IMAGE_IMPLEMENTATION
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    return OPENGL_ERROR_OK;
}

pge::IShader* pge::OpenglRenderer::create_shader(ShaderList shaders)
{
    auto id = m_shaders.size();

    auto &iter = m_shaders.emplace_back();

    iter.create(shaders);

    return &iter;
}

size_t pge::OpenglRenderer::create_mesh(std::span<float> data, std::array<std::string_view, 2> textures)
{
    OpenGlMesh mesh;

    create_texture(textures[0], mesh.texture1);
    create_texture(textures[1], mesh.texture2);

    mesh.vertex_size = data.size() / 5;

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    // vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    auto idx = m_meshes.size();

    m_meshes.emplace_back(mesh);

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

    auto shader = mesh.shader_params->shader;

    if (shader == nullptr)
    {
        goto draw;
    }

    shader->use();

    if (auto params = mesh.shader_params; params)
    {
        shader->set("enable_color", params->color_enabled);
        shader->set("enable_textures", params->textures_enabled);
        shader->set("object_color", params->object_color);
        shader->set("light_color", params->light_color);
        shader->set("texture_mix_value", params->texture_mix);

        if (params->light_source)
        {
            shader->set("light_pos", *params->light_source);
        }
    }

    shader->set("texture1", 0);
    shader->set("texture2", 1);
    shader->set("model", transform);
    shader->set("projection", m_camera->projection);
    shader->set("view", m_camera->view);
    shader->set("view_pos", m_camera->position);

draw:
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh.texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh.texture2);

    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_size);

    Engine::statistics.report_draw_call();
    Engine::statistics.report_verticies(mesh.vertex_size);

    return OPENGL_ERROR_OK;
}

void pge::OpenglRenderer::set_shader_params(ShaderParams *params, size_t mesh_id)
{
    if (mesh_id >= m_meshes.size())
    {
        return;
    }

    auto &mesh = m_meshes[mesh_id];

    mesh.shader_params = params;
}

uint32_t pge::OpenglRenderer::create_texture(std::string_view path, uint32_t &out_texture)
{
    stbi_set_flip_vertically_on_load(true);
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    auto format = channels > 3 ? GL_RGBA : GL_RGB;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
