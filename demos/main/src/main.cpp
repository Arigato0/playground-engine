#include <bits/stl_algo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <any>
#include <thread>

#include "application/engine.hpp"
#include "game/ecs.hpp"
#include "application/imgui_handler.hpp"
#include "application/input.hpp"
#include "application/misc.hpp"
#include "common_util/random.hpp"
#include "data/id_table.hpp"
#include "events/signal.hpp"
#include "graphics/CameraData.hpp"
#include "graphics/primitives.hpp"
#include "game/camera_comp.hpp"
#include "graphics/light.hpp"

using namespace pge;

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

class MeshRenderer : public IComponent
{
public:

    ~MeshRenderer()
    {
        if (m_model)
        {
            Engine::asset_manager.free_asset(m_path);
        }
    }

    void update(double delta_time) override
    {
        if (m_model == nullptr)
        {
            return;
        }

        for (const auto &mesh : m_model->meshes)
        {
            auto result = Engine::renderer->draw(mesh, m_parent->transform.model);

            if (result != 0)
            {
                Logger::fatal("could not draw mesh");
            }
        }
    }

    void set_mesh(std::string_view path)
    {
        if (m_model != nullptr)
        {
            Engine::asset_manager.free_asset(m_path);
        }

        m_model = Engine::asset_manager.get_model(path);

        m_path = path;
    }

    EditorProperties editor_properties() override
    {
        if (m_model == nullptr || m_model->meshes.empty())
        {
            return {};
        }

        EditorProperties properties;

        properties.reserve(m_model->meshes.size());

        for (auto &mesh : m_model->meshes)
        {
            auto &material = mesh.material;

            EditorProperties prop
            {
                {"Recieve light", &material.recieve_lighting},
                {"Color", ColorEdit(glm::value_ptr(material.color))},
                {"Shininess", DragControl(&material.shininess)},
                {"Texture scale",  DragControl(&material.diffuse.scale)},
                {"Enable texture", &material.diffuse.enabled}
            };

            util::concat(properties, prop);
        }

        return properties;
    }

    Model* get_model() const
    {
        return m_model;
    }

private:
    Model *m_model = nullptr;
    std::string_view m_path;
};

void render_properties(const EditorProperties &properties)
{
    for (const auto &prop : properties)
    {
        std::visit(overload
            {
                [&prop](bool *value)
                {
                    ImGui::Checkbox(prop.name.data(), value);
                },
                [&prop](Drag3Control<float> value)
                {
                    ImGui::DragFloat3(prop.name.data(), value.value, value.speed, value.min, value.max, value.format);
                },
                [&prop](DragControl<float> value)
                {
                    ImGui::DragFloat(prop.name.data(), value.value, value.speed, value.min, value.max, value.format);
                },
                [&prop](ColorEdit<float> value)
               {
                   ImGui::ColorEdit3(prop.name.data(), value.value);
               },
                [&prop](ButtonControl button)
                {
                    if (ImGui::Button(prop.name.data()))
                    {
                        button();
                    }
                },
            }, prop.control);
    }
}

class PlayerController : public IComponent
{
public:
    void on_start() override
    {
        m_camera = m_parent->find<CameraComp>();

        auto [window_width, window_height] = Engine::window.framebuffer_size();

        last_x = window_width / 2;
        last_y = window_height / 2;

        //Engine::window.set_cursor(CursorMode::Disabled);
    }
    void update(double delta_time) override
    {
        if (auto cords = get_mouse(); cords && !camera_locked)
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

        m_camera->data.yaw += x_offset;
        m_camera->data.pitch += y_offset;

        m_camera->data.pitch = std::clamp(m_camera->data.pitch, -90.0f, 90.0f);

        if (auto scroll = get_scroll(); scroll)
        {
            m_camera->data.zoom += scroll->y;
        }
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

    void on_start() override
    {
        m_player = Engine::entity_manager.find("Player");
        m_camera = m_player->find<CameraComp>();
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

            for (auto &[name, entity] : Engine::entity_manager.get_entities())
            {
                if (ImGui::TreeNode(name.data()))
                {
                    if (ImGui::TreeNode("Transform"))
                    {
                        auto &trans = entity.transform;

                        auto pos = trans.get_position();
                        auto scale = trans.get_scale();
                        auto euler = glm::eulerAngles(glm::quat_cast(trans.model));

                        if (ImGui::DragFloat3("Position", glm::value_ptr(pos)))
                        {
                            trans.set_position(pos);
                        }
                        if (ImGui::DragFloat3("Scale", glm::value_ptr(scale)))
                        {
                            trans.set_scale(scale);
                        }
                        ImGui::Text("Rotation");
                        if (ImGui::SliderAngle("X", &euler.x))
                        {
                            trans.model = glm::mat4{1.0f} * glm::eulerAngleXYZ(euler.x, 0.0f, 0.0f);
                        }
                        if (ImGui::SliderAngle("Y", &euler.y))
                        {
                            trans.model = glm::mat4{1.0f} * glm::eulerAngleXYZ(0.0f, euler.y, 0.0f);
                        }
                        if (ImGui::SliderAngle("Z", &euler.z))
                        {
                            trans.model = glm::mat4{1.0f} * glm::eulerAngleXYZ(0.0f, 0.0f, euler.z);
                        }

                        // if (ImGui::DragFloat3("Rotation", glm::value_ptr(euler)))
                        // {
                        //     trans.model = glm::rotate(trans.model, glm::radians(1.0f), euler);
                        // }

                        ImGui::TreePop();
                    }

                    // TODO the second component cant be toggled off for some reason
                    for (auto &[comp_name, comp] : entity.get_components())
                    {
                        auto is_enabled = comp->is_enabled();

                        if (ImGui::Checkbox("-", &is_enabled))
                        {
                            comp->set_enabled(is_enabled);
                        }

                        ImGui::SameLine();

                        if (ImGui::TreeNode(comp_name.data()))
                        {
                            auto properties = comp->editor_properties();
                            render_properties(properties);
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

                    static float fov = m_camera->data.fov;

                    if (ImGui::SliderFloat("FOV", &fov, 1, 135))
                    {
                        m_camera->data.fov = fov;
                    }

                    static float camera_near = m_camera->data.near;
                    if (ImGui::SliderFloat("Near", &camera_near, 0.01, 1000))
                    {
                        m_camera->data.near = camera_near;
                    }

                    static float camera_far = m_camera->data.near;

                    if (ImGui::SliderFloat("Far", &camera_far, 0.01, 1000))
                    {
                        m_camera->data.far = camera_far;
                    }

                    ImGui::DragFloat("Zoom", &m_camera->data.zoom, 0.1);

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
    Entity *m_player;
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

class LightComp : public IComponent
{
public:
    void on_start() override
    {
        data.position = &m_parent->transform.position;
        Light::table.emplace_back(&data);
    }

    void on_disable() override
    {
        data.is_active = false;
    }

    void on_enable() override
    {
        data.is_active = true;
    }

    std::vector<EditorProperty> editor_properties() override
    {
        return
        {
            {"Spotlight", &data.is_spot},
            {"Color", ColorEdit(glm::value_ptr(data.color))},
            {"Power", DragControl(&data.power)},
            {"Ambient", DragControl(&data.ambient)},
            {"diffuse", DragControl(&data.diffuse)},
            {"specular", DragControl(&data.specular)},
        };
    }

    Light data;
};

class ControlTest : public IComponent
{
public:
    bool show_window = false;
    float f_value = 10.0f;
    glm::vec3 my_vec3;

    // std::vector<EditorProperty> editor_properties() override
    // {
    //     return
    //     {
    //         {"Enabled", &show_window},
    //         {"My vec3", Drag3Control(glm::value_ptr(my_vec3))},
    //         {"Drag", DragControl(&f_value)},
    //         {"Hello", hello}
    //     };
    // }

    static void hello()
    {
        Logger::info("hello!");
    }

    void update(double delta_time) override
    {
        if (!show_window)
        {
            return;
        }

        ImGui::Begin("Test window", &show_window);

        ImGui::InputFloat3("my vec3", glm::value_ptr(my_vec3));
        ImGui::InputFloat("my vec3", &f_value);

        ImGui::End();
    }
};

struct Test
{
    ~Test()
    {
        fmt::println("destructor called");
    }
};
int main()
{
    ASSERT_ERR(Engine::init({
            .title = "playground engine",
            .window_size = {1920, 1080},
            .graphics_api = GraphicsApi::OpenGl,
        }));

    Engine::entity_manager.create<ControlTest>("ControlTest");
    Engine::entity_manager.create<InputHandlerComp, DebugUiComp>("Debug Editor");

    auto light_ent = Engine::entity_manager.create<LightComp>("Light");

    light_ent->transform.translate({2, 2, -1});
    light_ent->transform.scale(glm::vec3{0.5});

    auto backpack_ent = Engine::entity_manager.create<MeshRenderer>("Backpack");

    backpack_ent->transform.translate(glm::vec3{0, 0, -3});

    auto backpack_mesh = backpack_ent->find<MeshRenderer>();

    backpack_mesh->set_mesh("assets/models/backpack/backpack.obj");

    // auto backpack_ent2 = Engine::entity_manager.create<MeshRenderer>("Backpack2");
    //
    // backpack_ent2->transform.translate(glm::vec3{4, 0, -3});
    //
    // auto backpack_mesh2 = backpack_ent2->find<MeshRenderer>();
    //
    // backpack_mesh2->set_mesh("assets/models/backpack/backpack.obj");

    auto player = Engine::entity_manager.create<PlayerController, CameraComp>("Player");

    ASSERT_ERR(Engine::run());

    Engine::shutdown();
}