#include "imgui_handler.hpp"

#include "log.hpp"

pge::GraphicsApi g_using_api;

void pge::init_imgui(IWindow* window, GraphicsApi api)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "pge_imgui.ini";

#if defined(PGE_IMGUI_USE_GLFW)
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window->handle(), true);
#endif

    if (api == GraphicsApi::OpenGl)
    {
        ImGui_ImplOpenGL3_Init();
    }

    g_using_api = api;
}

void pge::cleanup_imgui()
{
    if (g_using_api == GraphicsApi::OpenGl)
    {
        ImGui_ImplOpenGL3_Shutdown();
    }

#if defined(PGE_IMGUI_USE_GLFW)
    ImGui_ImplGlfw_Shutdown();
#endif

    ImGui::DestroyContext();
}

void pge::imgui_new_frame()
{
    if (g_using_api == GraphicsApi::OpenGl)
    {
        ImGui_ImplOpenGL3_NewFrame();
    }

#if defined(PGE_IMGUI_USE_GLFW)
    ImGui_ImplGlfw_NewFrame();
#endif

    ImGui::NewFrame();
}

void pge::imgui_draw()
{
    ImGui::Render();

    if (g_using_api == GraphicsApi::OpenGl)
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
