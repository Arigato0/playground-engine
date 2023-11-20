#include "opengl_manager.hpp"

#include <imgui.h>

#define STB_IMAGE_IMPLEMENTATION
#include <bits/stl_algo.h>

#include "stb_image.h"

#include "glad/glad.h"
#include "opengl_error.hpp"
#include "opengl_shader.hpp"
#include "../../application/dialog.hpp"
#include "GLFW/glfw3.h"
#include "../../application/log.hpp"
#include "../../application/time.hpp"
#include "../../common_util/macros.hpp"

#define WINDOW_PTR (GLFWwindow*)m_window->handle()

static float CUBE_MESH[] =
{
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

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

pge::OpenGlErrorCode set_texture(uint32_t &texture, std::string_view path)
{
    auto tex_result = create_texture(path);

    if (!tex_result.ok())
    {
        texture = create_texture("assets/missing.jpeg").get();
        return tex_result.error();
    }

    texture = tex_result.get();

    return pge::OPENGL_ERROR_OK;
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

    glEnable(GL_DEPTH_TEST);

    stbi_set_flip_vertically_on_load(true);

    set_texture(m_texture, "assets/mona.jpg");
    set_texture(m_texture2, "assets/container.jpg");

    static uint32_t vbo, ebo;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_MESH), CUBE_MESH, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    auto trans = glm::mat4(1.0f);

    return OPENGL_ERROR_OK;
}

uint8_t pge::OpenGlManager::draw_frame()
{
    glfwSwapBuffers(WINDOW_PTR);

    glClearColor(EXPAND_VEC4(m_clear_color));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("Object Control");

    ImGui::SeparatorText("Texture");

    if (ImGui::Button("Load texture"))
    {
        auto path = native_file_dialog("~");
        glDeleteTextures(1, &m_texture);
        VALIDATE_ERR(set_texture(m_texture, path->c_str()));
    }

    if (ImGui::Button("Load Overlay"))
    {
        auto path = native_file_dialog("~");
        glDeleteTextures(1, &m_texture2);
        VALIDATE_ERR(set_texture(m_texture2, path->c_str()));
    }

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
        m_shader.set("color", glm::make_vec4(color));
    }

    static float mix_value = 0;
    ImGui::SliderFloat("Texture mix", &mix_value, 0, 1);

    auto time = program_time();

    static bool automatic_fade = false;
    ImGui::Checkbox("Automatic fade", &automatic_fade);

    if (automatic_fade)
    {
        static float fade_speed = 1;
        ImGui::SliderFloat("Fade speed", &fade_speed, 0.1, 5);
        mix_value = std::clamp(sin(time) * fade_speed, 0.0, 1.0);
    }

    m_shader.set("texture_mix_value", mix_value);

    m_shader.set("texture1", 0);
    m_shader.set("texture2", 1);

    static bool invert = false;
    static float offset[2] {0.f, 0.f};

    ImGui::SeparatorText("Transform");

    ImGui::Checkbox("Invert", &invert);
    ImGui::SliderFloat2("Offset", offset, -2, 2);

    glm::vec3 off_vector (glm::make_vec2(offset), 0.f);

    if (invert)
    {
        off_vector = -off_vector;
    }

    auto model = glm::mat4(1.0f);
    model = glm::translate(model, off_vector);

    static float rotation = 0;
    static float scale = 1;

    ImGui::SliderAngle("rotation", &rotation);
    ImGui::SliderFloat("Scale", &scale, 0, 100);

    static bool rotate_x = true, rotate_y, rotate_z;

    ImGui::Text("Rotation Axis");

    ImGui::Checkbox("x", &rotate_x);
    ImGui::SameLine();
    ImGui::Checkbox("y", &rotate_y);
    ImGui::SameLine();
    ImGui::Checkbox("z", &rotate_z);

    static bool automatic_rotation = false;
    ImGui::Checkbox("Automatic", &automatic_rotation);

    if (automatic_rotation)
    {
        static float speed = 2;
        rotation = time * speed;
        ImGui::SliderFloat("Speed", &speed, 0.0, 5);
    }

    // model = glm::rotate(model, rotation, glm::vec3(rotate_x, rotate_y, rotate_z));
    // model = glm::scale(model, glm::vec3(scale, scale, scale));

    static float view_vec[3] {0.f, 0.f, -3.0f};

    ImGui::SliderFloat3("View", view_vec, -10, 10);

    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::make_vec3(view_vec));

    auto [window_width, window_height] = m_window->framebuffer_size();

    static float fov = 65.0f;

    ImGui::SliderFloat("FOV", &fov, -360, 360);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(fov), (float)(window_width / window_height), 0.1f, 100.0f);

    //m_shader.set("model", model);
    m_shader.set("projection", projection);
    m_shader.set("view", view);

    static glm::vec3 cube_positions[] =
    {
        {0.0f,  0.0f,  0.0f},
        { 2.0f,  5.0f, -15.0f},
        {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f},
        { 2.4f, -0.4f, -3.5f},
        {-1.7f,  3.0f, -7.5f},
        { 1.3f, -2.0f, -2.5f},
        { 1.5f,  2.0f, -2.5f},
        { 1.5f,  0.2f, -1.5f},
        {-1.3f,  1.0f, -1.5f}
    };

    ImGui::End();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2);
    glBindVertexArray(m_vao);

    for (int i = 0; i < 10; i++)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, cube_positions[i]);

        auto angle = 3.0f * (i + 1);

        model = glm::rotate(model, glm::radians(float(time * angle)), glm::vec3(1.0f, 0.3f, 0.5f));

        m_shader.set("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

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
