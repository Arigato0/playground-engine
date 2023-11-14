#include <any>

#include "application/Application.hpp"
#include "game/entity.hpp"

int main()
{

    pge::Application app
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