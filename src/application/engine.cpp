#include "engine.hpp"

#include "./time.hpp"
#include "imgui_handler.hpp"
#include "input.hpp"
#include "../graphics/openGL/opengl_renderer.hpp"

pge::ErrorCode pge::Engine::init(AppInfo info)
{
    window.set_graphics_api(info.graphics_api);
	window.set_resizable(true);

    auto ok = window.open(info.title, info.window_size.x, info.window_size.y);

    if (!ok)
    {
        return ErrorCode::WindowCouldNotOpen;
    }

    init_input();

    set_graphics_api(info.graphics_api);

    auto result = renderer->init();

    if (result != 0)
    {
        Logger::warn("could not initialize graphics subsystem. {}", renderer->error_message(result));
        return ErrorCode::GraphicsSubsystemInitError;
    }

    renderer->set_clear_color({0.f, 0.f, 0.f, 1.f});

    auto properties = renderer->properties();

	Logger::info("using renderer {}", properties.to_string());

    init_imgui(&window, info.graphics_api);

    srand(time(0));

    m_initialized = true;

    return ErrorCode::Ok;
}

pge::ErrorCode pge::Engine::run()
{
    if (!m_initialized)
    {
        return ErrorCode::EngineNotInitialized;
    }

    entity_manager.start();

    while (!window.should_close())
    {
        glfwPollEvents();

        statistics.calculate();

		// per second logic goes here
		if (statistics.one_second_elapsed())
		{
			auto monitors_called = fs_monitor.poll();

			if (monitors_called)
			{
				Logger::info("called {} monitors", monitors_called);
			}
		}

        imgui_new_frame();

        entity_manager.update(statistics.delta_time());

        renderer->new_frame();

        renderer->end_frame();

        imgui_draw();

        reset_input();

        window.swap_buffers();
    }

	renderer->wait();

    return ErrorCode::Ok;
}

void pge::Engine::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    cleanup_imgui();

    delete renderer;

    m_initialized = false;
}

void pge::Engine::set_graphics_api(GraphicsApi api)
{
    delete renderer;

    using enum GraphicsApi;

    switch (api)
    {
        //case Vulkan: m_graphics_manager = new VulkanManager(); break;
        case OpenGl: renderer = new OpenglRenderer(); break;
    }
}