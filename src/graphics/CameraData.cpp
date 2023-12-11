#include "CameraData.hpp"

#include "../application/engine.hpp"

void pge::CameraData::process()
{
    static glm::vec3 direction {};

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(direction);
    right = glm::normalize(glm::cross(front, up));
    view  = glm::lookAt(position, position + front, up);
    view  = glm::scale(view, glm::vec3{zoom});

    if (type == Perspective)
    {
        auto [window_width, window_height] = Engine::window.framebuffer_size();

        projection = glm::perspective(glm::radians(fov), (float)(window_width / window_height), near, far);
    }
    else if (type == Ortographic)
    {
        projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near, far);
    }
}
