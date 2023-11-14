#include <complex>

#include "Application.hpp"
#include "../graphics/openGL/opengl_manager.hpp"
#include "./time.hpp"

#include "imgui_handler.hpp"

pge::ErrorCode pge::Application::init()
{
    m_window.set_graphics_api(m_info.graphics_api);
	m_window.resizable(true);

    auto ok = m_window.open(m_info.title, m_info.window_size.x, m_info.window_size.y);

    if (!ok)
    {
        return ErrorCode::WindowCouldNotOpen;
    }

    set_graphics_api(m_info.graphics_api);

    m_graphics_manager->set_window(&m_window);
	m_graphics_manager->set_clear_color({1.f, 0.0f, .3f, 1.f});

    auto result = m_graphics_manager->init();

    if (result != 0)
    {
        Logger::warn("could not initialize graphics subsystem. {}", m_graphics_manager->error_message(result));
        return ErrorCode::GraphicsSubsystemInitError;
    }

    auto properties = m_graphics_manager->properties();

	Logger::info("using renderer {}", properties.to_string());

    init_imgui(&m_window, m_info.graphics_api);

    return ErrorCode::Ok;
}

pge::ErrorCode pge::Application::run()
{
    auto previous_time = program_time();
    uint32_t frames = 0;

    while (!m_window.should_close())
    {
        if (m_window.is_key_pressed(Key::Escape))
        {
            break;
        }

        auto current_time = program_time();

        m_delta_time = current_time - previous_time;
        ++frames;

        if (m_delta_time > 1.0)
        {
            m_fps = double(frames) / m_delta_time;
            frames = 0;
            previous_time = current_time;
        }

		// auto time = glfwGetTime();
		// float color = std::sin(time) / 2.f + 0.5f;
		//
		// m_graphics_manager->set_clear_color({0.f, 0.3, color, 1.f});

        draw_ui();

		auto result = m_graphics_manager->draw_frame();

		if (result != 0)
		{
			Logger::warn("error while drawing frame: {}", m_graphics_manager->error_message(result));
		}

        glfwPollEvents();
    }

	m_graphics_manager->wait();

    return ErrorCode::Ok;
}

pge::Application::~Application()
{
    cleanup_imgui();
    delete m_graphics_manager;
	glfwTerminate();
}

void pge::Application::set_graphics_api(GraphicsApi api)
{
    delete m_graphics_manager;

    using enum GraphicsApi;

    switch (api)
    {
        //case Vulkan: m_graphics_manager = new VulkanManager(); break;
        case OpenGl: m_graphics_manager = new OpenGlManager(); break;
    }
}

void pge::Application::draw_ui()
{
    imgui_new_frame();

    static bool enable_wireframe = false;
    static bool settings_window_open = false;
    static bool stats_window_open = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close", "Escape"))
            {
                m_window.set_should_close(true);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::Checkbox("Settings", &settings_window_open);
            ImGui::Checkbox("Statistics", &stats_window_open);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (stats_window_open)
    {
        ImGui::Begin("Statistics", &stats_window_open);
        ImGui::Text(fmt::format("fps: {}", m_fps).data());
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

                m_graphics_manager->draw_wireframe(enable_wireframe);

                ImGui::Separator();
                ImGui::Text("Viewport");

                auto clear_color_vec = m_graphics_manager->get_clear_color();
                static float colors[3] {clear_color_vec.x, clear_color_vec.y, clear_color_vec.z};

                if (ImGui::ColorEdit3("Clear color", colors))
                {
                    m_graphics_manager->set_clear_color({colors[0], colors[1], colors[2], 1.f});
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

    imgui_draw();
}
