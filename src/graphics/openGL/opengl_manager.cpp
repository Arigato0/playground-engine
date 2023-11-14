#include "opengl_manager.hpp"

#include <array>

#include "glad/glad.h"
#include "opengl_error.hpp"
#include "opengl_shader.hpp"
#include "GLFW/glfw3.h"
#include "../../application/log.hpp"
#include "../../common_util/macros.hpp"

#define WINDOW_PTR (GLFWwindow*)m_window->handle()

void framebuffer_resize_cb(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
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

    auto vertex_shader = create_opengl_shader(PGE_FIND_SHADER("shader.vert"), ShaderType::Vertex);
    auto fragment_shader = create_opengl_shader(PGE_FIND_SHADER("shader.frag"), ShaderType::Fragment);

    m_shader_program = glCreateProgram();

    glAttachShader(m_shader_program, *vertex_shader);
    glAttachShader(m_shader_program, *fragment_shader);
    glLinkProgram(m_shader_program);

    glDeleteShader(*vertex_shader);
    glDeleteShader(*fragment_shader);

    float vertices[] =
    {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };

    uint32_t indices[] =
    {
        0, 1, 3,
        1, 2, 3,
    };

    static uint32_t vbo, ebo;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return OPENGL_ERROR_OK;
}

uint8_t pge::OpenGlManager::draw_frame()
{
    glfwSwapBuffers(WINDOW_PTR);

    glClearColor(EXPAND_VEC4(m_clear_color));
    glClear(GL_COLOR_BUFFER_BIT);


    glUseProgram(m_shader_program);
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
