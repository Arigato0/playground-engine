#include "input.hpp"

#include "imgui_handler.hpp"

#include "engine.hpp"

static glm::vec2 g_mouse_cords;
static bool g_mouse_called;

static glm::vec2 g_scroll_offsets;
static bool g_scroll_called;

struct KeyEvent
{
    bool called;
    int action;
    int mods;
};

static std::array<KeyEvent, 119> g_key_cache;

void mouse_callback(GLFWwindow *_, double xpos, double ypos)
{
    auto &io = ImGui::GetIO();

    io.AddMousePosEvent(xpos, ypos);

    if (io.WantCaptureMouse)
    {
        return;
    }

    g_mouse_cords =
    {
        xpos,
        ypos
    };

    g_mouse_called = true;
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    g_scroll_offsets =
    {
        xoffset,
        yoffset
    };

    g_scroll_called = true;
}

uint32_t translate_glfw_key(int key)
{
    // keys that dont follow a range of more than 3
    switch (key)
    {
        case GLFW_KEY_SPACE: return 0;
        case GLFW_KEY_APOSTROPHE: return 1;
        case GLFW_KEY_SEMICOLON: return 15;
        case GLFW_KEY_EQUAL: return 16;
        case GLFW_KEY_LEFT_BRACKET: return 43;
        case GLFW_KEY_BACKSLASH: return 44;
        case GLFW_KEY_RIGHT_BRACKET: return 45;
        case GLFW_KEY_GRAVE_ACCENT: return 46;
        case GLFW_KEY_WORLD_1: return 47;
        case GLFW_KEY_WORLD_2: return 48;
        case GLFW_KEY_MENU: return 118;
    }

    if (key >= GLFW_KEY_COMMA && key <= GLFW_KEY_9)
    {
        return 1 + (key - GLFW_KEY_COMMA);
    }
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
    {
        return 17 + (key - GLFW_KEY_A);
    }
    if (key >= GLFW_KEY_ESCAPE && key <= GLFW_KEY_END)
    {
        return 49 + (key - GLFW_KEY_ESCAPE);
    }
    if (key >= GLFW_KEY_CAPS_LOCK && key <= GLFW_KEY_PAUSE)
    {
        return 63 + (key - GLFW_KEY_CAPS_LOCK);
    }
    if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_KP_EQUAL)
    {
        return 69 + (key - GLFW_KEY_F1);
    }
    if (key >= GLFW_KEY_LEFT_SHIFT && key <= GLFW_KEY_RIGHT_SUPER)
    {
        return 110 + (key - GLFW_KEY_LEFT_SHIFT);
    }

    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto index = translate_glfw_key(key);
    auto &key_event = g_key_cache[index];

    key_event.called = true;
    key_event.action = action;
    key_event.mods = mods;
}

void pge::init_input()
{
    glfwSetCursorPosCallback((GLFWwindow*)Engine::window.handle(), mouse_callback);
    glfwSetKeyCallback((GLFWwindow*)Engine::window.handle(), key_callback);
    glfwSetScrollCallback((GLFWwindow*)Engine::window.handle(), scroll_callback);
}

void pge::reset_input()
{
    g_mouse_called = false;
    g_scroll_called = false;

    for (auto &key : g_key_cache)
    {
        key.called = false;
    }
}

std::optional<glm::vec2> pge::get_mouse()
{
    if (!g_mouse_called)
    {
        return std::nullopt;
    }

    return g_mouse_cords;
}

std::optional<glm::vec2> pge::get_scroll()
{
    if (!g_scroll_called)
    {
        return std::nullopt;
    }

    return g_scroll_offsets;
}

bool get_key(int key, int action, int mod)
{
    auto index = translate_glfw_key((int)key);
    auto key_event = g_key_cache[index];

    if (key_event.called && key_event.action == action && key_event.mods == mod)
    {
        return true;
    }

    return false;
}

bool pge::key_pressed(Key key, Modifier mod)
{
    return get_key(int(key), GLFW_PRESS, int(mod));
}

bool pge::key_held(Key key)
{
    return Engine::window.is_key_held(key);
}

bool pge::key_released(Key key, Modifier mod)
{
    return get_key(int(key), GLFW_RELEASE, int(mod));
}

std::string pge::key_name(Key key)
{
    auto *str = glfwGetKeyName(int(key), 0);

    if (str == nullptr)
    {
        return "Unknown key";
    }

    return str;
}
