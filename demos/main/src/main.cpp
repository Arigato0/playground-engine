#include <bits/stl_algo.h>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <any>
#include <imgui_internal.h>
#include <map>
#include <thread>

#include "application/platform/dialog.hpp"
#include "application/engine.hpp"
#include "game/ecs.hpp"
#include "application/imgui_handler.hpp"
#include "application/input.hpp"
#include "application/misc.hpp"
#include "common_util/random.hpp"
#include "data/id_table.hpp"
#include "events/signal.hpp"
#include "graphics/Camera.hpp"
#include "graphics/primitives.hpp"
#include "game/camera_comp.hpp"
#include "graphics/light.hpp"
#include "graphics/util.hpp"
#include "data/hash_table.hpp"
#include "application/platform/fs_monitor.hpp"
#include "application/platform/fs_events.hpp"

#include "misc/cpp/imgui_stdlib.h"

class LightComp;
using namespace pge;

class InputHandlerComp : public IComponent
{
public:
    void update(double delta_time) override
    {
        if (key_pressed(Key::Escape))
        {
            Engine::window.set_should_close(true);
        }
        if(key_held(Key::F5))
        {
           screen_shot("screen_shot");
        }
    }
};

PGE_COMPONENT(MeshRenderer)
{
public:

    MeshRenderer()
    {
        options.outline.color = glm::vec4{0.3, 0.05, 0.6, 1.0};
    }

    ~MeshRenderer() override
    {
        if (model.meshes.empty())
        {
            return;
        }

        Engine::asset_manager.free_asset(m_path);
    }

    void update(double delta_time) override
    {
        for (const auto &mesh : model.meshes)
        {
            Engine::renderer->draw(mesh, m_parent->transform.model, options);
        }
    }

    void editor_update(double delta_time) override
    {
        update(delta_time);
    }

    bool set_mesh(std::string_view path)
    {
        if (!model.meshes.empty())
        {
            Engine::asset_manager.free_asset(m_path);
        }

        auto model_opt = Engine::asset_manager.get_model(path);

        if (!model_opt)
        {
            return false;
        }

        model = std::move(model_opt.value());
		m_parent->transform.model = model.transform;
        m_path  = path;

        return true;
    }

    EditorProperties editor_properties() override
    {
        if (model.meshes.empty())
        {
            return
            {
                {
                    "Load mesh",
                    [&]
                    {
                        auto path = native_file_dialog("./assets/models/");

                        if (!path)
                        {
                            return;
                        }

                        set_mesh(path->c_str());
                    }
                }
            };
        }

        EditorProperties properties
        {
            {"Enable outline", &options.enable_outline},
            {"Outline thickness", DragControl(&options.outline.line_thickness)}
        };

        auto id = 0;

        for (auto &mesh : model.meshes)
        {
            auto &material = mesh.material;

            EditorProperties prop
            {
                START_GROUP,
                SEPERATOR(mesh.name),
                {"Recieve light", BitFlag(&material.flags, MAT_RECEIVE_LIGHT)},
				{"Cast shadow", BitFlag(&material.flags, MAT_CAST_SHADOW)},
				{"Contribute bloom", BitFlag(&material.flags, MAT_CONTRIBUTE_BLOOM)},
                {"Color", ColorEdit(glm::value_ptr(material.color))},
                {"Shininess", DragControl(&material.shininess)},
				{"Emission", DragControl(&material.emission)},
                {"Is transparent", BitFlag(&material.flags, MAT_USE_ALPHA)},
                {"Transparency", DragControl(&material.alpha)},
				{"Specular", DragControl(&material.specular)},
                {"Texture scale",  DragControl(&material.diffuse.scale)},
                {"Enable texture", &material.diffuse.enabled},
				{"Enable normals", &material.bump.enabled},
				{"Enable depth", &material.depth.enabled},
				{"Depth Strength", DragControl(&material.depth_strength)},
				{"Normal strength", DragControl(&material.bump_strength)},
				{"Flip normals", BitFlag(&material.flags, MAT_FLIP_NORMALS)},
                {"Set Diffuse", [&mesh]
                {
                    auto path = native_file_dialog("~");

                    if (!path)
                    {
                        return;
                    }

                    mesh.material.diffuse = *Engine::asset_manager.get_texture(path->c_str());
                }},
                END_GROUP
            };

            util::concat(properties, prop);
        }

        return properties;
    }

    DrawOptions options;

    ModelView model;
private:
    std::string m_path;
};

void render_properties(const EditorProperties &properties)
{
    auto id = 0;

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
            [&prop](SeperatorControl _)
            {
                ImGui::SeparatorText(prop.name.data());
            },
            [&id](StartGroup _)
            {
                ImGui::PushID(id++);
            },
            [](EndGroup _)
            {
                ImGui::PopID();
            },
			[&prop](BitFlag bf)
            {
                bool enabled = *bf.flag & bf.mask;

				ImGui::Checkbox(prop.name.data(), &enabled);

				if (enabled)
				{
					*bf.flag |= bf.mask;
				}
				else
				{
					*bf.flag &= ~bf.mask;
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

        // ImGui::Begin("Game view", 0);
        //
        // auto *framebuffer = Engine::renderer->get_framebuffer();
        //
        // ImGui::Image(ImTextureID(framebuffer->get_texture()), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
        //
        // ImGui::End();
        //
        // ImGui::DockSpaceOverViewport();

        if (show_object_control)
        {
            ImGui::Begin("Objects", &show_object_control);

            // delete later to avoid corrupting stuff
            std::vector<std::string_view> entities_to_delete;
            std::vector<std::pair<Entity*, std::string_view>> components_to_delete;

            static auto modal_open = false;

            if (ImGui::Button("New"))
            {
                modal_open = true;
                ImGui::OpenPopup("Create new entity");
            }

            if (ImGui::BeginPopupModal("Create new entity", &modal_open, ImGuiWindowFlags_AlwaysAutoResize))
            {
                static std::string name;

                ImGui::InputText("Name", &name);

                if (ImGui::Button("Create") && !name.empty())
                {
                    modal_open = false;
                    Engine::entity_manager.create(std::move(name));
					name.clear();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::Separator();

            for (auto &[name, entity] : Engine::entity_manager.get_entities())
            {
                if (ImGui::TreeNode(name.data()))
                {
                    if (ImGui::Button("Add"))
                    {
                        ImGui::OpenPopup("Add component");
                    }

                    if (ImGui::BeginPopup("Add component"))
                    {
                        auto &prototypes = Engine::entity_manager.get_comp_prototypes();

                        for (auto &[name, comp] : prototypes)
                        {
                            if (ImGui::Selectable(name.data()))
                            {
                                auto new_comp = entity.add_component_prototype(name, comp.get());
								new_comp->on_start();
                            }
                        }

                        ImGui::EndPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Delete"))
                    {
                        entities_to_delete.push_back(name);
                    }

                    ImGui::Separator();

                    if (ImGui::TreeNode("Transform"))
                    {
                        auto &trans = entity.transform;

                        auto pos = trans.get_position();
                        auto scale = trans.get_scale();
                        auto euler = glm::eulerAngles(glm::quat_cast(trans.model));
                        auto euler_old = euler;
						static auto all_scale = 0.0f;

                        if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1))
                        {
                            trans.set_position(pos);
                        }
						if (ImGui::DragFloat("Scale xyz", &all_scale, 0.1))
                        {
                            trans.set_scale(all_scale);
                        }
                        if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1))
                        {
                            trans.set_scale(scale);
                        }

                        ImGui::Text("Rotation");
                        if (ImGui::DragFloat("X", &euler.x, 0.5))
                        {
                            trans.model = glm::rotate(trans.model, glm::radians(euler_old.x - euler.x), {1, 0, 0});
                        }
                        if (ImGui::DragFloat("Y", &euler.y, 0.5))
                        {
                            trans.model = glm::rotate(trans.model, glm::radians(euler_old.y - euler.y), {0, 1, 0});
                        }
                        if (ImGui::DragFloat("Z", &euler.z, 0.5))
                        {
                            trans.model = glm::rotate(trans.model, glm::radians(euler_old.z - euler.z), {0, 0, 1});
                        }

                        ImGui::TreePop();
                    }

					int id = 0;

                    for (auto &[comp_name, comp] : entity.get_components())
                    {
                        auto is_enabled = comp->is_enabled();

						ImGui::PushID(id++);
                        if (ImGui::Checkbox("##", &is_enabled))
                        {
                            comp->set_enabled(is_enabled);
                        }
						ImGui::PopID();

                        ImGui::SameLine();

                        if (ImGui::TreeNode(comp_name.data()))
                        {
                            auto properties = comp->editor_properties();
                            render_properties(properties);
                            ImGui::TreePop();
                        }

                        ImGui::SameLine();

						ImGui::PushID(id++);
                        if (ImGui::Button("Delete"))
                        {
                            components_to_delete.emplace_back(&entity, comp_name);
                        }
						ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
            }

            for (auto &[entity, name] : components_to_delete)
            {
                entity->remove_component(name);
            }
            for (auto &name : entities_to_delete)
            {
                Engine::entity_manager.erase(name);
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
			auto render_stats = Engine::renderer->get_stats();

            ImGui::Begin("Statistics", &stats_window_open);

            auto str = fmt::format("fps: {}\nFrame time: {}\ndraw calls: {}\nvertices: {}",
                stats.fps, stats.delta_time, render_stats.draw_calls, render_stats.vertices);

            ImGui::Text(str.data());

            ImGui::End();
        }

        if (settings_window_open)
        {
            ImGui::Begin("Settings", &settings_window_open);
            if (ImGui::BeginTabBar("Tabs"))
            {
                if (ImGui::BeginTabItem("Rendering"))
                {
					ImGui::SeparatorText("Antialiasing");

					static int msaa_samples = 4;

					if (ImGui::SliderInt("MSAA samples", &msaa_samples, 0, 32))
					{
						Engine::renderer->get_render_framebuffer()->set_samples(msaa_samples);
					}

#define CHECK_CHANGE(var, exp) do { if ((exp)) { (var) = true; } } while(false)

					{
						ImGui::SeparatorText("Shadows");

						auto changed = false;
						auto settings = Engine::renderer->get_shadow_settings();

						CHECK_CHANGE(changed, ImGui::Checkbox("Soft Shadows", &settings.enable_soft));
						CHECK_CHANGE(changed, ImGui::DragInt("PCF samples", &settings.pcf_samples));
						CHECK_CHANGE(changed, ImGui::DragFloat("Bias", &settings.bias, 0.1));

						if (changed)
						{
							Engine::renderer->set_shadow_settings(settings);
						}
					}

					{
						ImGui::SeparatorText("Textures");

						auto changed = false;
						auto settings = Engine::renderer->get_texture_settings();

						CHECK_CHANGE(changed, ImGui::DragFloat("anisotropic level", &settings.anisotropic_level, 1, 0));

						if (changed)
						{
							Engine::renderer->set_texture_settings(settings);
						}
					}

					ImGui::SeparatorText("Debug Visualize");

                    ImGui::Checkbox("Wireframe", &enable_wireframe);

                    Engine::renderer->set_wireframe_mode(enable_wireframe);

					static bool visualize_depth = false;

                    if (ImGui::Checkbox("Visualize depth buffer", &visualize_depth))
                    {
                        Engine::renderer->set_visualize_depth(visualize_depth);
                    }

                    ImGui::SeparatorText("Viewport");

                    static auto clear_color_vec = Engine::renderer->get_clear_color();

                    if (ImGui::ColorEdit4("Clear color", glm::value_ptr(clear_color_vec)))
                    {
                        Engine::renderer->set_clear_color(clear_color_vec);
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

                    static float camera_far = m_camera->data.far;

                    if (ImGui::SliderFloat("Far", &camera_far, 0.01, 1000))
                    {
                        m_camera->data.far = camera_far;
                    }

                    ImGui::DragFloat("Zoom", &m_camera->data.zoom, 0.1);

                    static bool framerate_cap = false;

					{
						ImGui::SeparatorText("Display");

						auto changed = false;
						auto settings = Engine::renderer->get_color_settings();

						CHECK_CHANGE(changed, ImGui::DragFloat("Gamma", &settings.gamma, 0.1));
						CHECK_CHANGE(changed, ImGui::DragFloat("Exposure", &settings.exposure, 0.1));
						CHECK_CHANGE(changed, ImGui::Checkbox("Bloom", &settings.enable_bloom));

						if (settings.enable_bloom)
						{
							CHECK_CHANGE(changed, ImGui::DragFloat("Bright threshold", &settings.bright_threshold, 0.1));
							CHECK_CHANGE(changed, ImGui::DragInt("Bloom blur passes", &settings.bloom_blur_passes, 0.1));
						}

						if (changed)
						{
							Engine::renderer->set_screen_space_settings(settings);
						}
					}

                    if (ImGui::Checkbox("Framerate cap", &framerate_cap))
                    {
                        Engine::window.cap_refresh_rate(framerate_cap);
                    }

                    static bool fullscreen = Engine::window.is_fullscreen();

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

PGE_COMPONENT(LightComp)
{
public:

    void on_init() override
    {
        data.position = &m_parent->transform.position;
        Light::table.emplace_back(&data);
    }

    ~LightComp() override
    {
        data.is_active = false;
        //Light::table.remove(&data);
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

	inline static constexpr uint8_t FLAG1 { 1 << 0 };
	inline static constexpr uint8_t FLAG2 { 1 << 2 };

    bool show_window = false;
    float f_value = 10.0f;
    glm::vec3 my_vec3;
	uint32_t flags;

    std::vector<EditorProperty> editor_properties() override
    {
        return
        {
            {"Enabled", &show_window},
            {"My vec3", Drag3Control(glm::value_ptr(my_vec3))},
            {"Drag", DragControl(&f_value)},
            {"Hello", hello},
			{"Flag1", BitFlag(&flags, FLAG1)}
        };
    }

    static void hello()
    {
        Logger::info("hello!");
    }

    void update(double delta_time) override
    {
        if (!(flags & FLAG1) || show_window)
        {
            return;
        }

        ImGui::Begin("Test window", &show_window);

        ImGui::InputFloat3("my vec3", glm::value_ptr(my_vec3));
        ImGui::InputFloat("my vec3", &f_value);

        ImGui::End();
    }
};

std::pair<Entity*, MeshRenderer*> create_mesh(std::string_view name, std::string_view path)
{
    auto entity = Engine::entity_manager.create<MeshRenderer>(name);

    auto mesh = entity->find<MeshRenderer>();

    mesh->set_mesh(path);

    return {entity, mesh};
}

void init_sponza_scene()
{
	auto light_ent = Engine::entity_manager.create<LightComp>("Light");

    light_ent->transform.translate({2, 15, -1});
	auto light = light_ent->find<LightComp>();

	light->data.power = 4;

	auto [sponza_ent, sponza_mesh] = create_mesh("sponza", "/home/arian/Downloads/glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf");
//	sponza_ent->transform.scale(glm::vec3{0.01});

	for (auto &mesh : sponza_mesh->model.meshes)
	{
//		mesh.material.depth.enabled = false;
//		mesh.material.bump.enabled = false;
		mesh.material.flags |= MAT_USE_ALPHA;
//		mesh.material.flags &= ~(MAT_RECEIVE_LIGHT | MAT_CAST_SHADOW);
	}
}

void init_room_scene()
{
    auto light_ent = Engine::entity_manager.create<LightComp>("Light");

    light_ent->transform.translate({2, 3, -1});

    create_mesh("Room", "/home/arian/Downloads/testing room/testing room 2.obj");

    auto [sword_ent, sword_mesh] = create_mesh("Sword", "/home/arian/Downloads/lowpoly-stylized-scimitar/source/scimitarobj.obj");

	auto &sword_mat = sword_mesh->model.meshes.front();

//	sword_mat.material.color = {0.969, 0.059, 0.106};
//	sword_mat.material.emission = 5;

    auto [window_ent, window_mesh] = create_mesh("Window", "assets/models/primitives/plane.glb");

    window_ent->transform.translate({0, 2, -3});
    window_ent->transform.rotate(180, glm::vec3{0, 1, 1});

    auto &window_material = window_mesh->model.meshes.front().material;
    window_material.diffuse = *Engine::asset_manager.get_texture("assets/window.png", true, TextureWrapMode::ClampToEdge);
    window_material.shininess = 1;
    window_material.flags |= MAT_USE_ALPHA;

//	create_mesh("table", "/home/arian/Downloads/wooden_table_02_4k.gltf/wooden_table_02_4k.gltf");
	auto [couch_ent, _] = create_mesh("couch", "/home/arian/Downloads/gaudy_couch/scene.gltf");
	couch_ent->transform.translate({-3.4, -0.8, -0.15});
	//create_mesh("conference room", "/home/arian/Downloads/conference/conference.obj");

	create_mesh("Wrench", "/home/arian/Documents/Adobe/Adobe Substance 3D Painter/export/scifi_wrench/scifi_wrench2.gltf");

    auto [skull_ent, skull_mesh] = create_mesh("Skull", "/home/arian/Downloads/scull-cup/source/SculCup/Cup_low.obj");

    skull_ent->transform.translate({-2, 1.5, -3});
    skull_ent->transform.rotate(30, {0, 1, 0});

//    for (auto &mesh : skull_mesh->model.meshes)
//    {
////        mesh.material.use_alpha = true;
////        mesh.material.alpha = 0.3f;
//        mesh.material.color = {1, 0.161, 0.933};
//		mesh.material.emission = 3;
//		mesh.material.cast_shadow = false;
//		mesh.material.bump.enabled = false;
//    }

	auto [wall_ent, wall_mesh_comp] = create_mesh("wall", "assets/models/primitives/plane.glb");

	wall_ent->transform.translate({0, 1, 0});
	wall_ent->transform.rotate(90, {1, 0, 0});
	wall_ent->transform.rotate(-180, {0, 0, 1});

	auto &wall_mesh = wall_mesh_comp->model.meshes.front();

	wall_mesh.material.diffuse = *Engine::asset_manager.get_texture("/home/arian/Downloads/wood.png");
	wall_mesh.material.bump = *Engine::asset_manager.get_texture("/home/arian/Downloads/toy_box_normal.png");
	wall_mesh.material.depth = *Engine::asset_manager.get_texture("/home/arian/Downloads/toy_box_disp.png");
	wall_mesh.material.depth_strength = 0.3;
}

PGE_COMPONENT(CameraViewComp)
{
public:

	CameraViewComp()
	{
		m_name = fmt::format("Camera View #{}", m_count++);
	}

	void update(double delta_time) override
	{
		m_data.process();

		ImGui::Begin(m_name.c_str());
		auto texture = m_view->framebuffer->get_texture();
		ImGui::Image(ImTextureID(texture), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}

	~CameraViewComp() override
	{
		if (m_view == nullptr)
		{
			return;
		}

		Engine::renderer->remove_view(m_view);
	}

	void on_start() override
	{
		m_view = Engine::renderer->add_view(&m_data);

		if (m_view == nullptr)
		{
			m_enabled = false;
		}
	}

private:
	Camera m_data {};
	RenderView *m_view = nullptr;
	std::string m_name;
	inline static int m_count = 0;
};

void run_engine()
{
	ASSERT_ERR(Engine::init({
            .title = "playground engine",
            .window_size = {1920, 1080},
            .graphics_api = GraphicsApi::OpenGl,
        }));

    Engine::entity_manager.create<ControlTest>("ControlTest");
    Engine::entity_manager.create<InputHandlerComp, DebugUiComp>("Debug Editor");

    Engine::renderer->set_clear_color({0.09, 0.871, 1, 0.902});

    register_components<MeshRenderer, LightComp>();

     std::array<std::string_view, 6> skybox_faces
     {
         "assets/skybox/Daylight Box_Pieces/Daylight Box_Right.bmp",
         "assets/skybox/Daylight Box_Pieces/Daylight Box_Left.bmp",
         "assets/skybox/Daylight Box_Pieces/Daylight Box_Top.bmp",
         "assets/skybox/Daylight Box_Pieces/Daylight Box_Bottom.bmp",
         "assets/skybox/Daylight Box_Pieces/Daylight Box_Front.bmp",
         "assets/skybox/Daylight Box_Pieces/Daylight Box_Back.bmp"
     };

     uint32_t skybox_texture;

     auto result = Engine::renderer->create_cubemap_from_path(skybox_faces, skybox_texture);

     assert(result == 0);

     Engine::renderer->set_skybox(skybox_texture);

    init_room_scene();
//	 init_sponza_scene();

// 	Engine::entity_manager.create<CameraViewComp>("CameraView");
//	Engine::entity_manager.create<CameraViewComp>("CameraView2");
//	Engine::entity_manager.create<CameraViewComp>("CameraView3");
//	Engine::entity_manager.create<CameraViewComp>("CameraView4");

/*
 * auto mesh = ...
 * mesh.set_mesh("./path");
 * glm::mat4 transforms[];
 * mesh.set_instances(transforms);
 */
    
    Engine::entity_manager.create<PlayerController, CameraComp>("Player");

    ASSERT_ERR(Engine::run());

    Engine::shutdown();
}

int main()
{
    run_engine();
}
