#include <complex>

#include "engine.hpp"
#include "window.hpp"
#include "../graphics/openGL/opengl_manager.hpp"
#include "./time.hpp"

#include "imgui_handler.hpp"

pge::ErrorCode pge::Engine::init(AppInfo info)
{
    window.set_graphics_api(info.graphics_api);
	window.resizable(true);

    auto ok = window.open(info.title, info.window_size.x, info.window_size.y);

    if (!ok)
    {
        return ErrorCode::WindowCouldNotOpen;
    }

    set_graphics_api(info.graphics_api);

    graphics_manager->set_window(&window);
	graphics_manager->set_clear_color({0.f, 0.5f, 0.6f, 1.f});

    auto result = graphics_manager->init();

    if (result != 0)
    {
        Logger::warn("could not initialize graphics subsystem. {}", graphics_manager->error_message(result));
        return ErrorCode::GraphicsSubsystemInitError;
    }

    auto properties = graphics_manager->properties();

	Logger::info("using renderer {}", properties.to_string());

    init_imgui(&window, info.graphics_api);

    return ErrorCode::Ok;
}

pge::ErrorCode pge::Engine::run()
{
    entity_manager.start();

    while (!window.should_close())
    {
        glfwPollEvents();

        statistics.calculate();

        imgui_new_frame();

        entity_manager.update(statistics.delta_time());

		auto result = graphics_manager->draw_frame();

		if (result != 0)
		{
			Logger::warn("error while drawing frame: {}", graphics_manager->error_message(result));
		}

        imgui_draw();
    }

	graphics_manager->wait();

    return ErrorCode::Ok;
}

void pge::Engine::shutdown()
{
    cleanup_imgui();
    delete graphics_manager;
	glfwTerminate();
}

void pge::Engine::set_graphics_api(GraphicsApi api)
{
    delete graphics_manager;

    using enum GraphicsApi;

    switch (api)
    {
        //case Vulkan: m_graphics_manager = new VulkanManager(); break;
        case OpenGl: graphics_manager = new OpenGlManager(); break;
    }
}