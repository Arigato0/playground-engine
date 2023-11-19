#include "opengl_manager.hpp"

#include <imgui.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glad/glad.h"
#include "opengl_error.hpp"
#include "opengl_shader.hpp"
#include "GLFW/glfw3.h"
#include "../../application/log.hpp"
#include "../../application/time.hpp"
#include "../../common_util/macros.hpp"

#define WINDOW_PTR (GLFWwindow*)m_window->handle()

void framebuffer_resize_cb(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

pge::Result<uint32_t, pge::OpenGlErrorCode> create_texture(std::string_view path)
{
    int width, height, channels;

    uint8_t *data = stbi_load(path.data(), &width, &height, &channels, 0);

    DEFER([&data]
    {
        stbi_image_free(data);
    });

    if (data == nullptr)
    {
        return pge::OPENGL_ERROR_TEXTURE_LOADING;
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

    return id;
}

uint8_t pge::OpenGlManager::init()
{
    auto result = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (!result)
    {
        return OPENGL_ERROR_GLAD_INIT;
    }

    auto [width, height] = m_window->framebuffer_size();

    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(WINDOW_PTR, framebuffer_resize_cb);

    m_shader.create(
    {
        {PGE_FIND_SHADER("shader.vert"), ShaderType::Vertex},
        {PGE_FIND_SHADER("shader.frag"), ShaderType::Fragment}
    });

    float vertices[] =
    {
        // positions          // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   // bottom right
       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
       -0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // top left
    };

    uint32_t indices[] =
    {
        0, 1, 3,
        1, 2, 3,
    };

    stbi_set_flip_vertically_on_load(true);
    auto tex_result = create_texture("assets/mona.jpg");

    if (!tex_result.ok())
    {
        return tex_result.error();
    }

    m_texture = tex_result.get();

    tex_result = create_texture("assets/awesomeface.png");

    if (!tex_result.ok())
    {
        return tex_result.error();
    }

    m_texture2 = tex_result.get();

    static uint32_t vbo, ebo;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return OPENGL_ERROR_OK;
}

uint8_t pge::OpenGlManager::draw_frame()
{
    glfwSwapBuffers(WINDOW_PTR);

    glClearColor(EXPAND_VEC4(m_clear_color));
    glClear(GL_COLOR_BUFFER_BIT);


    ImGui::Begin("Object Control");

    static auto enable_texture = true;
    static auto enable_color = false;

    ImGui::Checkbox("Texture", &enable_texture);

    if (!enable_texture)
    {
        glDisableVertexAttribArray(1);
    }
    else
    {
        glEnableVertexAttribArray(1);
    }

    ImGui::Checkbox("Color", &enable_color);

    m_shader.set("enable_color", enable_color);

    m_shader.use();
    static float color[4] { 1.0, 1.0, 1.0, 1.0};

    if (ImGui::ColorEdit4("Color", color))
    {
        m_shader.set("color", {color[0], color[1], color[2], color[3]});
    }

    static float mix_value = 0.3;
    ImGui::SliderFloat("Texture mix", &mix_value, 0, 1);

    m_shader.set("texture_mix_value", mix_value);

    m_shader.set("texture1", 0);
    m_shader.set("texture2", 1);

    static float x_off;
    static float y_off;
    static bool invert = false;
    ImGui::SliderFloat("X offset", &x_off, -1, 1);
    ImGui::SliderFloat("Y offset", &y_off, -1, 1);
    ImGui::Checkbox("Invert", &invert);

    m_shader.set("x_off", invert ? -x_off : x_off);
    m_shader.set("y_off", invert ? -y_off : y_off);

    ImGui::End();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2);
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    return OPENGL_ERROR_OK;
}

// TODO im pretty sure the strings this returns arent static so they become invalid as soon as the function exits
std::vector<const char *> pge::OpenGlManager::extensions()
{
    std::vector<const char *> output;

    GLint count;

    glGetIntegerv(GL_NUM_EXTENSIONS, &count);

    output.resize(count);

    for (int i = 0; i < count; i++)
    {
        auto value = (const char*)glGetStringi(GL_EXTENSIONS, i);

        if (value == nullptr)
        {
            continue;
        }

        output.push_back(value);
    }

    return output;
}
