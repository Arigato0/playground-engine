#include "camera.hpp"

#include "../application/engine.hpp"

void pge::Camera::update()
{
    static glm::vec3 direction {};

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    forward = glm::normalize(direction);

    auto [window_width, window_height] = Engine::window.framebuffer_size();

    view = glm::lookAt(position, position + forward, up);
    projection = glm::perspective(glm::radians(fov), (float)(window_width / window_height), near, far);
}
