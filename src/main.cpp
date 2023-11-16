#include <any>

#include "application/engine.hpp"
#include "game/entity.hpp"

class Test : pge::Entity
{
public:

    Test()
    {
        Logger::info("Test constructed");
    }

    void update(double delta_time) override
    {
        Logger::info("updated");
    }

    PGE_MAKE_SERIALIZABLE();
};

int main()
{
    pge::Engine app
    (
        {
            .title = "playground engine",
            .window_size = {720, 460},
            .graphics_api = pge::GraphicsApi::OpenGl,
        }
    );

    ASSERT_ERR(app.init());
    ASSERT_ERR(app.run());
}