#include "glfw_window.hpp"


void window_close_callback(GLFWwindow *glfw_window)
{
    auto ptr = (pge::IWindow*)glfwGetWindowUserPointer(glfw_window);
    ptr->on_close(ptr);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    auto ptr = (pge::IWindow*)glfwGetWindowUserPointer(window);
    ptr->on_resize(ptr, width, height);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto ptr = (pge::IWindow*)glfwGetWindowUserPointer(window);
    ptr->on_framebuffer_resize(ptr, width, height);
}

void window_pos_callback(GLFWwindow* window, int xpos, int ypos)
{
    auto ptr = (pge::IWindow*)glfwGetWindowUserPointer(window);
    ptr->on_position(ptr, xpos, ypos);
}

void window_iconify_callback(GLFWwindow* window, int iconified)
{
    auto ptr = (pge::IWindow*)glfwGetWindowUserPointer(window);
    ptr->on_iconify(ptr, iconified);
}

void window_maximize_callback(GLFWwindow* window, int maximized)
{
    auto ptr = (pge::IWindow*)glfwGetWindowUserPointer(window);
    ptr->on_maximize(ptr, maximized);
}

void pge::GlfwWindow::set_callbacks()
{
    glfwSetWindowCloseCallback(m_window, window_close_callback);
    glfwSetWindowSizeCallback(m_window, window_size_callback);
    glfwSetWindowSizeCallback(m_window, framebuffer_size_callback);
    glfwSetWindowPosCallback(m_window, window_pos_callback);
    glfwSetWindowIconifyCallback(m_window, window_iconify_callback);
    glfwSetWindowMaximizeCallback(m_window, window_maximize_callback);

    on_framebuffer_resize.connect(
        [&](IWindow*, int width, int height)
        {
            m_width = width;
            m_height = height;
        });
}

pge::GlfwWindow::GlfwWindow()
{
    if (!m_is_init)
    {
        glfwInit();
        m_is_init = true;
    }

    glfwSetErrorCallback(glfw_error_cb);
}

bool pge::GlfwWindow::open(std::string_view title, int width, int height)
{
    if (!m_is_init)
    {
        glfwInit();
        m_is_init = true;
    }

    m_window = glfwCreateWindow(width, height, title.data(), m_monitor, nullptr);

    glfwMakeContextCurrent(m_window);

    set_raw_input(true);

    if (m_window)
    {
        glfwSetWindowUserPointer(m_window, this);
        set_callbacks();
    }

    m_width = width;
    m_height = height;

    return m_window != nullptr;
}

void pge::GlfwWindow::set_title(std::string_view title)
{
    glfwSetWindowTitle(m_window, title.data());
}

void pge::GlfwWindow::resize(int width, int height)
{
    glfwSetWindowSize(m_window, width, height);
}

void pge::GlfwWindow::change(int width, int height, int refresh_rate, int xpos, int ypos)
{
    glfwSetWindowMonitor(m_window, m_monitor, xpos, ypos, width, height, refresh_rate);
}

void pge::GlfwWindow::cap_refresh_rate(bool value)
{
    glfwSwapInterval(value);
}

void pge::GlfwWindow::set_fullscreen(bool value)
{
    auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    int xpos, ypos;
    glfwGetWindowPos(m_window, &xpos, &ypos);

    if (value && m_monitor == nullptr)
    {
        m_monitor = glfwGetPrimaryMonitor();
    }
    if (!value)
    {
        m_monitor = nullptr;
    }

    glfwSetWindowMonitor(m_window,
        m_monitor, xpos, ypos, mode->width, mode->height, mode->refreshRate);
}

void pge::GlfwWindow::close()
{
    glfwDestroyWindow(m_window);
    m_window = nullptr;
}

bool pge::GlfwWindow::should_close()
{
    return glfwWindowShouldClose(m_window);
}

void pge::GlfwWindow::set_should_close(bool value)
{
    glfwSetWindowShouldClose(m_window, value);
}

pge::GlfwWindow::~GlfwWindow()
{
    close();
    glfwTerminate();
    m_is_init = false;
}

void* pge::GlfwWindow::handle()
{
    return m_window;
}

void pge::GlfwWindow::set_resizable(bool value)
{
    glfwWindowHint(GLFW_RESIZABLE, value);
}

void pge::GlfwWindow::set_graphics_api(GraphicsApi api)
{
    assert(is_implemented(api) && "unimplemented graphics api supplied");

    using enum GraphicsApi;

    switch (api)
    {
        case Vulkan:
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
        }
        case OpenGl:
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            break;
        }
    }
}

std::pair<uint32_t, uint32_t> pge::GlfwWindow::framebuffer_size()
{
    return { m_width, m_height };
}

bool pge::GlfwWindow::is_key_held(Key key)
{
    return glfwGetKey(m_window, (int)key) == GLFW_PRESS;
}

glm::dvec2 pge::GlfwWindow::mouse_xy()
{
    glm::dvec2 output;

    glfwGetCursorPos(m_window, &output.x, &output.y);

    return output;
}

void pge::GlfwWindow::set_cursor(CursorMode mode)
{
    glfwSetInputMode(m_window, GLFW_CURSOR, (int)mode);
}

void pge::GlfwWindow::set_raw_input(bool value)
{
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, value);
    }
}

bool pge::GlfwWindow::is_fullscreen() const
{
    return m_monitor != nullptr;
}

void pge::GlfwWindow::swap_buffers()
{
    glfwSwapBuffers(m_window);
}

void pge::GlfwWindow::glfw_error_cb(int code, const char* description)
{
    Logger::warn("glfw error [{}]: {}", code, description);
}

