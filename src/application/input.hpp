#pragma once

#include <optional>

#include "glm/glm.hpp"

namespace pge
{
    void init_input();
    void reset_input();

    std::optional<glm::vec2> mouse_cords();
}
