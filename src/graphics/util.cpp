#include "util.hpp"
#include "../application/engine.hpp"

void pge::screen_shot(std::string_view path, IFramebuffer* framebuffer)
{
    Image img;

    if (framebuffer != nullptr)
    {
       img = framebuffer->get_image();
    }
    else
    {
        img = Engine::renderer->get_image();
    }

    img.save(path);
}
