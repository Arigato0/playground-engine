#pragma once

#include <optional>

#include "input_keys.hpp"
#include "glm/glm.hpp"

namespace pge
{
    void init_input();
    void reset_input();

    std::optional<glm::vec2> mouse_cords();

    bool key_pressed(Key key, Modifier mod = Modifier::None);
    bool key_held(Key key, Modifier mod = Modifier::None);

    std::string key_name(Key key);

}
