cmake_minimum_required(VERSION 3.23)
project(playgroundEngine)

set(CMAKE_CXX_STANDARD 20)

#set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#
#set(IMGUI_DIR  "/lib/imgui-docking")
#
#add_subdirectory(lib/glfw-3.3.8)
include_directories(lib/glad/include/ lib/imgui-docking/ lib/imgui-docking/backends)
add_subdirectory("lib")

add_executable(playgroundEngine
        src/main.cpp
        lib/glad/src/glad.c
        src/application/log.hpp
        src/application/fmt.hpp
        src/application/glfw_window.cpp
        src/application/glfw_window.hpp
        src/application/Application.cpp
        src/application/Application.hpp
        src/application/error.hpp
        src/application/error.cpp
        src/graphics/vulkan/queue_families.hpp
        src/graphics/vulkan/queue_families.cpp
        src/graphics/vulkan/vulkan_manager.cpp
        src/graphics/vulkan/vulkan_manager.hpp
        src/graphics/vulkan/swap_chain.cpp
        src/graphics/vulkan/swap_chain.hpp
        src/common_util/io.cpp
        src/common_util/io.hpp
        src/graphics/vulkan/util.hpp
        src/common_util/data_structure.hpp
        src/common_util/defer.hpp
        src/common_util/constraints.hpp
        src/common_util/macros.hpp
        src/common_util/util.hpp
        src/common_util/validation.hpp
        src/graphics/vulkan/vulkan_shader.cpp
        src/graphics/vulkan/vulkan_shader.hpp
        src/graphics/vulkan/util.hpp
        src/application/log.cpp
        src/graphics/graphics_manager_interface.hpp
        src/graphics/vulkan/graphics_errors.cpp
        src/graphics/vulkan/graphics_errors.hpp
        src/common_util/debug.hpp
        src/graphics/graphics_api.hpp
        src/application/window_interface.hpp
        src/application/window.hpp
        src/graphics/openGL/opengl_manager.cpp
        src/graphics/openGL/opengl_manager.hpp
        src/graphics/openGL/opengl_error.hpp
        src/graphics/openGL/opengl_error.cpp
        src/application/input_keys.hpp
        src/graphics/shaders.hpp
        src/application/imgui_handler.cpp
        src/application/imgui_handler.hpp
        src/graphics/openGL/opengl_shader.cpp
        src/graphics/openGL/opengl_shader.hpp
        src/graphics/types.hpp
        src/game/entity.hpp
        src/application/time.hpp)

target_link_libraries(playgroundEngine glfw vulkan ${OPENGL_LIBRARIES} IMGUI)
