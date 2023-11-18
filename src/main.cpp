#include <any>
#include <atomic>
#include <thread>

#include "application/engine.hpp"
#include "game/ecs.hpp"
#include "application/imgui_handler.hpp"

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
                }

                ImGui::Checkbox("Settings", &settings_window_open);
                ImGui::Checkbox("Statistics", &stats_window_open);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (stats_window_open)
        {
            ImGui::Begin("Statistics", &stats_window_open);
            ImGui::Text(fmt::format("fps: {}", Engine::statistics.fps()).data());
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

                    Engine::graphics_manager->draw_wireframe(enable_wireframe);

                    ImGui::Separator();
                    ImGui::Text("Viewport");

                    auto clear_color_vec = Engine::graphics_manager->get_clear_color();
                    static float colors[3]{clear_color_vec.x, clear_color_vec.y, clear_color_vec.z};

                    if (ImGui::ColorEdit3("Clear color", colors))
                    {
                        Engine::graphics_manager->set_clear_color({colors[0], colors[1], colors[2], 1.f});
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

int main()
{
    ASSERT_ERR(Engine::init({
            .title = "playground engine",
            .window_size = {720, 460},
            .graphics_api = pge::GraphicsApi::OpenGl,
        }));

    Engine::entity_manager.create<DebugEditor, InputHandlerComp, DebugUiComp>("Debug Editor");

    ASSERT_ERR(Engine::run());

    Engine::shutdown();
}