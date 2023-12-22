#pragma once

#include <string_view>

#include "framebuffer_interface.hpp"
#include "image.hpp"

namespace pge
{
    void screen_shot(std::string_view path, IFramebuffer *framebuffer = nullptr);
}
