#include "input.hpp"

#include <imgui.h>

#include "engine.hpp"

static glm::vec2 g_mouse_cords;
static bool g_called;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    ImGuiIO& io = ImGui::GetIO();

    io.AddMousePosEvent(xpos, ypos);

    g_mouse_cords.x = xpos;
    g_mouse_cords.y = ypos;

    g_called = true;
}

void pge::init_input()
{
    glfwSetCursorPosCallback((GLFWwindow*)Engine::window.handle(), mouse_callback);
}

void pge::reset_input()
{
    g_called = false;
}

std::optional<glm::vec2> pge::mouse_cords()
{
    if (!g_called)
    {
        return std::nullopt;
    }

    return g_mouse_cords;
}
