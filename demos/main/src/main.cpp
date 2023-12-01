#include <bits/stl_algo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/compatibility.hpp>

#include "application/engine.hpp"
#include "game/ecs.hpp"
#include "application/imgui_handler.hpp"
#include "application/input.hpp"
#include "application/misc.hpp"
#include "graphics/camera.hpp"
#include "graphics/primitives.hpp"

using namespace pge;

class DebugEditor : public IEntity
{};

class InputHandlerComp : public IComponent
{
public:
    void update(double delta_time) override
    {
        if (Engine::window.is_key_held(Key::Escape))
        {
            Engine::window.set_should_close(true);
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
        Engine::renderer->set_shader_params(&params, m_mesh_id);
    }

    ShaderParams params;

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
        Engine::renderer->set_camera(&camera);
    }
    void update(double delta_time) override
    {
        camera.update();
    }

    void move_forward(float mod = 1)
    {
        camera.position += speed * camera.front * mod;
    }

    void move_backward(float mod = 1)
    {
        camera.position -= speed * camera.front * mod;
    }

    void move_right(float mod = 1)
    {
        camera.position += glm::normalize(glm::cross(camera.front, camera.up)) * speed * mod;
    }

    void move_left(float mod = 1)
    {
        camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) * speed * mod;
    }

    void move_up(float mod = 1)
    {
        camera.position += camera.up * speed * mod;
    }

    void move_down(float mod = 1)
    {
        camera.position -= camera.up * speed * mod;
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

        Engine::window.set_cursor(CursorMode::Disabled);
    }
    void update(double delta_time) override
    {
        if (auto cords = mouse_cords(); cords && !camera_locked)
        {
            handle_mouse(delta_time, cords->x, cords->y);
        }
        handle_movement(delta_time);
        other_input();
    }

    void other_input()
    {
        if (key_pressed(Key::C))
        {
            show_cursor = !show_cursor;
            Engine::window.set_cursor(show_cursor ? pge::CursorMode::Normal : pge::CursorMode::Disabled);
        }
        if (key_pressed(Key::C, Modifier::Shift))
        {
            camera_locked = !camera_locked;
        }
    }

    void handle_movement(double delta_time)
    {
        auto mod = delta_time * speed_mod;

        if (key_held(Key::W))
        {
            m_camera->move_forward(mod);
        }
        if (key_held(Key::S))
        {
            m_camera->move_backward(mod);
        }
        if (key_held(Key::A))
        {
            m_camera->move_left(mod);
        }
        if (key_held(Key::D))
        {
            m_camera->move_right(mod);
        }
        if (key_held(Key::Space))
        {
            m_camera->move_up(mod);
        }
        if (key_held(Key::LeftControl))
        {
            m_camera->move_down(mod);
        }

        if (key_held(Key::LeftShift))
        {
            speed_mod = 4;
        }
        else
        {
            speed_mod = 1;
        }
    }

    void handle_mouse(double delta_time, float x, float y)
    {
        auto x_offset = x - last_x;
        auto y_offset = last_y - y;

        last_x = x;
        last_y = y;

        x_offset *= mouse_sensitivity;
        y_offset *= mouse_sensitivity;

        m_camera->camera.yaw += x_offset;
        m_camera->camera.pitch += y_offset;

        m_camera->camera.pitch = std::clamp(m_camera->camera.pitch, -90.0f, 90.0f);
    }

    float speed_mod = 1.0f;
    float mouse_sensitivity = 0.1f;
private:
    CameraComp *m_camera;
    bool camera_locked = false;
    float last_x;
    float last_y;
    bool show_cursor = false;
};

class DebugUiComp : public IComponent
{
public:
    bool enable_wireframe     = false;
    bool show_all             = false;
    bool settings_window_open = false;
    bool stats_window_open    = false;
    bool demo_window_open     = false;
    bool show_object_control  = false;

    MeshRenderer *ground_mesh;

    void on_start() override
    {
        m_player = (Player*)Engine::entity_manager.find("Player");
        m_camera = m_player->find<CameraComp>();

        auto ground_ent = Engine::entity_manager.find("Ground");

        ground_mesh = ground_ent->find<MeshRenderer>();
    }

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
                ImGui::Checkbox("Objects", &show_object_control);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (show_object_control)
        {
            ImGui::Begin("Objects", &show_object_control);

            for (const auto &[name, entity] : Engine::entity_manager.get_entities())
            {
                if (ImGui::TreeNode(name.data()))
                {
                    if (ImGui::TreeNode("Transform"))
                    {
                        auto &trans = entity->transform;

                        auto *pos = glm::value_ptr(trans.position);

                        if (ImGui::SliderFloat3("Position", pos, -1000, 1000, "%.2f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat))
                        {
                            auto vec3 = glm::make_vec3(pos);
                            trans.transform[3] = {vec3, 1.0f};
                        }

                        ImGui::TreePop();
                    }

                    for (const auto &[comp_name, comp] : entity->get_components())
                    {
                        if (ImGui::TreeNode(comp_name.data()))
                        {
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
            }

            ImGui::End();
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

                    ImGui::SeparatorText("Viewport");

                    auto clear_color_vec = Engine::renderer->clear_color;
                    static float colors[3]{clear_color_vec.x, clear_color_vec.y, clear_color_vec.z};

                    if (ImGui::ColorEdit3("Clear color", colors))
                    {
                        Engine::renderer->clear_color = {colors[0], colors[1], colors[2], 1.f};
                    }

                    ImGui::SeparatorText("Camera");

                    static float fov = m_camera->camera.fov;

                    if (ImGui::SliderFloat("FOV", &fov, 1, 135))
                    {
                        m_camera->camera.fov = fov;
                    }

                    static float camera_near = m_camera->camera.near;
                    if (ImGui::SliderFloat("Near", &camera_near, 0.01, 1000))
                    {
                        m_camera->camera.near = camera_near;
                    }

                    static float camera_far = m_camera->camera.near;

                    if (ImGui::SliderFloat("Far", &camera_far, 0.01, 1000))
                    {
                        m_camera->camera.far = camera_far;
                    }

                    static bool framerate_cap = false;

                    if (ImGui::Checkbox("Framerate cap", &framerate_cap))
                    {
                        Engine::window.cap_refresh_rate(framerate_cap);
                    }

                    static bool fullscreen = true;

                    if (ImGui::Checkbox("Fullscreen", &fullscreen))
                    {
                        Engine::window.set_fullscreen(fullscreen);
                    }

                    if (ImGui::TreeNode("Set resolution"))
                    {
                        static int current_item = 0;
                        auto resolution_strings = get_resolution_strings();

                        std::vector<const char*> raw_strings;

                        raw_strings.reserve(resolution_strings.size());

                        for (auto &str : resolution_strings)
                        {
                            raw_strings.emplace_back(str.c_str());
                        }

                        if (ImGui::ListBox("resolution", &current_item, raw_strings.data(), raw_strings.size()))
                        {
                            auto modes = get_video_modes();
                            auto m = modes[current_item];
                            Engine::window.change(m.width, m.height, m.refresh_rate);
                        }

                        ImGui::TreePop();
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
private:
    CameraComp *m_camera;
    Player *m_player;
};

class ObjectRotator : public IComponent
{
public:

    void update(double delta_time) override
    {
        m_parent->transform.rotate((sin(3) * 3) * Engine::time_scale, {1, 1, 1});
        m_parent->transform.translate({0, (cos(program_time() * 3) * 0.020), 0});
    }
};

class FlashLight : public IComponent
{
public:

    void on_start() override
    {
        m_light = Engine::entity_manager.find("Light");
    }

    void update(double delta_time) override
    {
        m_light->transform.transform = m_parent->transform.transform;
    }

private:
    IEntity *m_light;
};

int main()
{
    ASSERT_ERR(Engine::init({
            .title = "playground engine",
            .window_size = {1920, 1080},
            .graphics_api = pge::GraphicsApi::OpenGl,
        }));

    Engine::entity_manager.create<DebugEditor, InputHandlerComp, DebugUiComp>("Debug Editor");

    auto lighting_shader = Engine::renderer->create_shader
    ({
        {PGE_FIND_SHADER("shader.vert"), ShaderType::Vertex},
        {PGE_FIND_SHADER("lighting.frag"), ShaderType::Fragment}
    });

    auto standard_shader = Engine::renderer->create_shader
    ({
       {PGE_FIND_SHADER("shader.vert"), ShaderType::Vertex},
       {PGE_FIND_SHADER("shader.frag"), ShaderType::Fragment},
   });

    auto light_ent = Engine::entity_manager.create<Cube, MeshRenderer, ObjectRotator>("Light");

    light_ent->transform.translate({0, 3, -10});
    light_ent->transform.scale(glm::vec3{0.5});

    auto light_mesh = light_ent->find<MeshRenderer>();

    if (light_mesh == nullptr)
    {
        Logger::fatal("Could not find component");
    }

    light_mesh->set_mesh(CUBE_MESH, {""});

    //cube_mesh->params.object_color = {1.0f, 0.5f, 0.31f};
    light_mesh->params.textures_enabled = false;
    light_mesh->params.color_enabled = true;
    light_mesh->params.shader = standard_shader;

    auto ground_ent = Engine::entity_manager.create<Cube, MeshRenderer>("Ground");

    ground_ent->transform.scale({100, 0.5, 100});
    ground_ent->transform.translate({0, -3, 0});

    auto ground_mesh = ground_ent->find<MeshRenderer>();

    ground_mesh->set_mesh(CUBE_MESH, {"assets/plaster.jpg"});

    ground_mesh->params =
    {
        .textures_enabled = false,
        .color_enabled = true,
        .object_color = {0.0f, 0.5f, 0.51f},
        .shader = lighting_shader,
        .light_pos = &light_ent->transform.position,
        .specular = {0.0f, 0.0f, 0.0f},
        .texture_scale = 10,
        .enable_specular = false,
        // .light_ambient = {0.8f, 0.1f, 0.2f},
        // .light_diffuse = {0.8f, 0.1f, 0.2f},
        // .light_specular = {0.1f, 0.1f, 0.1f},
    };

    auto box_ent = Engine::entity_manager.create<Cube, MeshRenderer>("Box");

    box_ent->transform.translate({-2, -0.25, -10});

    auto box_mesh = box_ent->find<MeshRenderer>();

    box_mesh->set_mesh(CUBE_MESH, {"assets/container2.png", "assets/container2_specular.png"});

    box_mesh->params =
    {
        .textures_enabled = true,
        .color_enabled = true,
        .object_color = {0.0f, 0.5f, 0.51f},
        .shader = lighting_shader,
        .light_pos = &light_ent->transform.position,
        .specular = {0.5f, 0.5f, 0.5f},
        // .light_ambient = {0.8f, 0.1f, 0.2f},
        // .light_diffuse = {0.8f, 0.1f, 0.2f},
        // .light_specular = {0.1f, 0.1f, 0.1f},
    };

    for (int i = 0; i < 100; i++)
    {
        glm::mat4 trans {1.0f};

        trans = glm::rotate(trans, glm::radians(20.0f * i), glm::vec3{0, 1, 0});
        trans = glm::translate(trans, glm::vec3{1 + i, -0.25, 1 + i});

        box_mesh->add_instance(trans);
    }

    // float offset = 10;
    //
    // for (int i = 0; i < 100; i++)
    // {
    //     for (int j = 0; j < 100; j++)
    //     {
    //         glm::mat4 trans {1.0f};
    //
    //         trans = glm::translate(trans, glm::vec3{j + offset, 0, i + offset});
    //
    //         cube_mesh->add_instance(trans);
    //     }
    // }

    auto player = Engine::entity_manager.create<Player, PlayerController, CameraComp>("Player");

    ASSERT_ERR(Engine::run());

    Engine::shutdown();

}