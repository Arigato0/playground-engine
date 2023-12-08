#pragma once

#include <optional>

#include "input_keys.hpp"
#include "glm/glm.hpp"

namespace pge
{
    void init_input();
    void reset_input();

    std::optional<glm::vec2> get_mouse();
    std::optional<glm::vec2> get_scroll();

    bool key_pressed(Key key, Modifier mod = Modifier::None);
    bool key_held(Key key);
    bool key_released(Key key, Modifier mod = Modifier::None);

    std::string key_name(Key key);

}
