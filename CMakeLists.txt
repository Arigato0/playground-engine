cmake_minimum_required(VERSION 3.23)
project(playgroundEngineLib)

set(CMAKE_CXX_STANDARD 20)

include_directories(lib/glad/include/ lib/imgui-docking/ lib/imgui-docking/backends lib/stb/ lib/glm/glm)
add_subdirectory("lib")
add_subdirectory("lib/glm/")

add_library(playgroundEngineLib
        lib/glad/src/glad.c
        lib/stb/stb_image.h
        src/application/log.hpp
        src/application/fmt.hpp
        src/application/glfw_window.cpp
        src/application/glfw_window.hpp
        src/application/engine.cpp
        src/application/engine.hpp
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
        src/graphics/vulkan/graphics_errors.cpp
        src/graphics/vulkan/graphics_errors.hpp
        src/common_util/debug.hpp
        src/graphics/graphics_api.hpp
        src/application/window_interface.hpp
        src/application/window.hpp
        src/graphics/openGL/opengl_error.hpp
        src/graphics/openGL/opengl_error.cpp
        src/application/input_keys.hpp
        src/graphics/shaders.hpp
        src/application/imgui_handler.cpp
        src/application/imgui_handler.hpp
        src/graphics/openGL/opengl_shader.cpp
        src/graphics/openGL/opengl_shader.hpp
        src/graphics/types.hpp
        src/game/ecs.hpp
        src/application/time.hpp
        src/game/entity_manager.hpp
        src/game/entity_manager.cpp
        src/application/statistics.cpp
        src/application/statistics.hpp
        src/application/dialog.cpp
        src/application/dialog.hpp
        src/graphics/renderer_interface.hpp
        src/graphics/mesh.hpp
        src/graphics/openGL/opengl_renderer.cpp
        src/graphics/openGL/opengl_renderer.hpp
        src/graphics/camera.cpp
        src/graphics/camera.hpp
        src/graphics/primitives.hpp
        src/game/transform.hpp
        src/application/input.cpp
        src/application/input.hpp
        src/application/misc.cpp
        src/application/misc.hpp
        src/common_util/misc.cpp
        src/common_util/misc.hpp
        src/events/signal.hpp
        src/graphics/light.hpp
)

target_link_libraries(playgroundEngineLib glfw vulkan ${OPENGL_LIBRARIES} IMGUI glm)
