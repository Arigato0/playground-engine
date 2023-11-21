#include <complex>

#include "engine.hpp"
#include "window.hpp"
#include "../graphics/openGL/opengl_manager.hpp"
#include "./time.hpp"

#include "imgui_handler.hpp"
#include "../graphics/openGL/opengl_renderer.hpp"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        static bool cursor_visible = true;
        pge::Engine::window.set_cursor(cursor_visible ? pge::CursorMode::Normal : pge::CursorMode::Disabled);
        cursor_visible = !cursor_visible;
    }
}

pge::ErrorCode pge::Engine::init(AppInfo info)
{
    window.set_graphics_api(info.graphics_api);
	window.resizable(true);

    auto ok = window.open(info.title, info.window_size.x, info.window_size.y);

    if (!ok)
    {
        return ErrorCode::WindowCouldNotOpen;
    }

    glfwSetKeyCallback((GLFWwindow*)window.handle(), key_callback);

    set_graphics_api(info.graphics_api);

    renderer->clear_color = {0.f, 0.5f, 0.6f, 1.f};

    auto result = renderer->init();

    if (result != 0)
    {
        Logger::warn("could not initialize graphics subsystem. {}", renderer->error_message(result));
        return ErrorCode::GraphicsSubsystemInitError;
    }

    auto properties = renderer->properties();

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

        renderer->new_frame();

        entity_manager.update(statistics.delta_time());

        imgui_draw();
    }

	renderer->wait();

    return ErrorCode::Ok;
}

void pge::Engine::shutdown()
{
    cleanup_imgui();
    delete renderer;
	glfwTerminate();
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