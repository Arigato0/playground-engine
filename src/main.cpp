#include <any>
#include <atomic>
#include <thread>
#include <bits/stl_algo.h>

#include "stb_image.h"
#include "application/dialog.hpp"
#include "application/engine.hpp"
#include "game/ecs.hpp"
#include "application/imgui_handler.hpp"
#include "graphics/camera.hpp"
#include "graphics/primitives.hpp"
#include "graphics/renderer_interface.hpp"
#include "graphics/openGL/opengl_renderer.hpp"

using namespace pge;

class DebugEditor : public IEntity
{};

class InputHandlerComp : public IComponent
{
public:
    void update(double delta_time) override
    {
        if (Engine::window.is_key_pressed(Key::Escape))
        {
            Engine::window.set_should_close(true);
        }
    }
};

class DebugUiComp : public IComponent
{
public:
    bool enable_wireframe     = false;
    bool show_all             = false;
    bool settings_window_open = false;
    bool stats_window_open    = false;
    bool demo_window_open     = false;

    void update(double delta_time) override
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Close", "Escape"))
                {
                    Engine::window.set_should_close(true);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Windows"))
            {
                ImGui::Checkbox("All", &show_all);

                if (show_all)
                {
                    settings_window_open = show_all;
                    stats_window_open    = show_all;
                    demo_window_open     = show_all;
                }

                ImGui::Checkbox("Settings", &settings_window_open);
                ImGui::Checkbox("Statistics", &stats_window_open);
                ImGui::Checkbox("Demo", &demo_window_open);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (demo_window_open)
        {
            ImGui::ShowDemoWindow(&demo_window_open);
        }

        if (stats_window_open)
        {
            auto stats = Engine::statistics.stats();
            ImGui::Begin("Statistics", &stats_window_open);
            ImGui::Text(fmt::format("fps: {}", stats.fps).data());
            ImGui::Text(fmt::format("draw calls: {}", stats.draw_calls).data());
            ImGui::Text(fmt::format("vertices: {}", stats.vertices).data());
            ImGui::End();
        }

        if (settings_window_open)
        {
            ImGui::Begin("Settings", &settings_window_open);
            if (ImGui::BeginTabBar("Tabs"))
            {
                if (ImGui::BeginTabItem("Renderering"))
                {
                    ImGui::Checkbox("Wireframe", &enable_wireframe);

                    Engine::renderer->set_wireframe_mode(enable_wireframe);

                    ImGui::Separator();
                    ImGui::Text("Viewport");

                    auto clear_color_vec = Engine::renderer->clear_color;
                    static float colors[3]{clear_color_vec.x, clear_color_vec.y, clear_color_vec.z};

                    if (ImGui::ColorEdit3("Clear color", colors))
                    {
                        Engine::renderer->clear_color = {colors[0], colors[1], colors[2], 1.f};
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Style"))
                {
                    ImGui::ShowStyleEditor();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();
        }
    }
};

class Cube : public IEntity
{

};

class MeshRenderer : public IComponent
{
public:

    void update(double delta_time) override
    {
        Engine::renderer->draw(m_mesh_id, m_parent->transform.transform);

        for (auto instance : m_instances)
        {
            Engine::renderer->draw(m_mesh_id, instance);
        }
    }

    void add_instance(glm::mat4 transform)
    {
        m_instances.push_back(transform);
    }

    void set_mesh(std::span<float> mesh, std::array<std::string_view, 2> textures)
    {
        m_mesh_id = Engine::renderer->create_mesh(mesh, textures);
    }
private:
    size_t m_mesh_id;
    std::vector<glm::mat4> m_instances;
};

class Player : public IEntity
{

};

class CameraComp : public IComponent
{
public:
    void on_start() override
    {
        camera.position.z = 3;
    }
    void update(double delta_time) override
    {
        camera.update();
    }

    void move_forward(float mod = 1)
    {
        camera.position += speed * camera.forward * mod;
    }

    void move_backward(float mod = 1)
    {
        camera.position -= speed * camera.forward * mod;
    }

    void move_right(float mod = 1)
    {
        camera.position += glm::normalize(glm::cross(camera.forward, camera.up)) * speed * mod;
    }

    void move_left(float mod = 1)
    {
        camera.position -= glm::normalize(glm::cross(camera.forward, camera.up)) * speed * mod;
    }

    float speed = 2.5f;
    Camera camera;
};


class PlayerController : public IComponent
{
public:
    void on_start() override
    {
        m_camera = m_parent->find<CameraComp>();

        auto [window_width, window_height] = Engine::window.framebuffer_size();

        last_x = window_width / 2;
        last_y = window_height / 2;
    }
    void update(double delta_time) override
    {
        //handle_mouse(delta_time);
        handle_movement(delta_time);
    }

    void handle_movement(double delta_time)
    {
        if (Engine::window.is_key_pressed(Key::W))
        {
            m_camera->move_forward(delta_time * speed_mod);
        }
        if (Engine::window.is_key_pressed(Key::S))
        {
            m_camera->move_backward(delta_time * speed_mod);
        }
        if (Engine::window.is_key_pressed(Key::A))
        {
            m_camera->move_left(delta_time * speed_mod);
        }
        if (Engine::window.is_key_pressed(Key::D))
        {
            m_camera->move_right(delta_time * speed_mod);
        }
    }

    void handle_mouse(float delta_time, float x, float y)
    {
        auto [window_width, window_height] = Engine::window.framebuffer_size();

        auto x_offset = x - last_x;
        auto y_offset = last_y - y;

        last_x = x;
        last_y = y;

        x_offset *= mouse_sensitivity;
        y_offset *= mouse_sensitivity;

        m_camera->camera.yaw += x_offset;
        m_camera->camera.pitch += y_offset;

        m_camera->camera.pitch = std::clamp(m_camera->camera.pitch, -89.0f, 89.0f);
    }

    float speed_mod = 1.0f;
    float mouse_sensitivity = 0.1f;
private:
    CameraComp *m_camera;
    float last_x;
    float last_y;
};

PlayerController *controller;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    ImGuiIO& io = ImGui::GetIO();

    io.AddMousePosEvent(xpos, ypos);

    controller->handle_mouse(Engine::statistics.delta_time(), xpos, ypos);
}

int main()
{
    ASSERT_ERR(Engine::init({
            .title = "playground engine",
            .window_size = {1920, 1080},
            .graphics_api = pge::GraphicsApi::OpenGl,
        }));

    glfwSetCursorPosCallback((GLFWwindow*)Engine::window.handle(), mouse_callback);

    Engine::entity_manager.create<DebugEditor, InputHandlerComp, DebugUiComp>("Debug Editor");

    auto cube_ent = Engine::entity_manager.create<Cube, MeshRenderer>("Cube");

    // cube_ent->transform.transform = glm::scale(cube_ent->transform.transform, glm::vec3{2.2});
    // cube_ent->transform.transform = glm::translate(cube_ent->transform.transform, glm::vec3{0, 0.5, 0});
    cube_ent->transform.rotate(60, {0, 1, 0});
    auto cube_mesh = cube_ent->find<MeshRenderer>();

    cube_mesh->set_mesh(CUBE_MESH, {"assets/mona.jpg"});

    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            glm::mat4 trans {1.0f};

            trans = glm::translate(trans, glm::vec3{j, 0, i});

            cube_mesh->add_instance(trans);
        }
    }

    auto player_ent = Engine::entity_manager.create<Player, PlayerController, CameraComp>("Player");

    controller = player_ent->find<PlayerController>();
    ASSERT_ERR(Engine::run());

    Engine::shutdown();

}