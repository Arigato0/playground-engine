#pragma once
#include "window_interface.hpp"

#define PGE_IMGUI_USE_GLFW

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace pge
{
    void init_imgui(IWindow *window, GraphicsApi api);
    void cleanup_imgui();
    void imgui_new_frame();
    void imgui_draw();
}
