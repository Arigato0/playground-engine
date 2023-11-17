#include <any>

#include "application/engine.hpp"
#include "game/ecs.hpp"

using namespace pge;

class InputHandler : public Entity
{
public:
    void update(double delta_time) override
    {
        if (Engine::window.is_key_held(Key::C))
        {
            // Engine::window.set_cursor(show_cursor ? CursorMode::Normal : CursorMode::Disabled);
            // Logger::info("changed cursor mode to {}", show_cursor);
            // show_cursor = !show_cursor;
        }
        if (Engine::window.is_key_pressed(Key::Escape))
        {
            Engine::window.set_should_close(true);
        }
    }

    PGE_MAKE_SERIALIZABLE();

private:
    bool show_cursor = true;
};

int main()
{
    ASSERT_ERR(Engine::init({
            .title = "playground engine",
            .window_size = {720, 460},
            .graphics_api = pge::GraphicsApi::OpenGl,
        }));


    InputHandler input_handler;

    pge::Engine::entity_manager.register_entity("input handler", &input_handler);

    ASSERT_ERR(Engine::run());

    Engine::shutdown();
}