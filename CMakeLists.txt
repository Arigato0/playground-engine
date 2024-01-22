cmake_minimum_required(VERSION 3.23)
project(playgroundEngineLib)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "-O3")
endif()

set(CMAKE_CXX_STANDARD 20)

include_directories(lib/glad/include/ lib/imgui-docking/ lib/imgui-docking/backends lib/stb/ lib/glm/glm lib/assimp/include
        lib/unordered_dense/include/ src/)
add_subdirectory("lib")
add_subdirectory("lib/glm/")
add_subdirectory("lib/assimp")
add_subdirectory("lib/unordered_dense")

add_library(playgroundEngineLib
        lib/glad/src/glad.c
        lib/stb/stb_image.h
        lib/stb/stb_image_write.h
        "src/application/log.hpp"
        "src/application/fmt.hpp"
        "src/application/glfw_window.cpp"
        "src/application/glfw_window.hpp"
        "src/application/engine.cpp"
        "src/application/engine.hpp"
        "src/application/error.hpp"
        "src/application/error.cpp"
        "src/graphics/vulkan/queue_families.hpp"
        "src/graphics/vulkan/queue_families.cpp"
        "src/graphics/vulkan/vulkan_manager.cpp"
        "src/graphics/vulkan/vulkan_manager.hpp"
        "src/graphics/vulkan/swap_chain.cpp"
        "src/graphics/vulkan/swap_chain.hpp"
        "src/common_util/io.cpp"
        "src/common_util/io.hpp"
        "src/graphics/vulkan/util.hpp"
        "src/common_util/data_structure.hpp"
        "src/common_util/defer.hpp"
        "src/common_util/constraints.hpp"
        "src/common_util/macros.hpp"
        "src/common_util/util.hpp"
        "src/common_util/validation.hpp"
        "src/graphics/vulkan/vulkan_shader.cpp"
        "src/graphics/vulkan/vulkan_shader.hpp"
        "src/graphics/vulkan/util.hpp"
        "src/application/log.cpp"
        "src/graphics/vulkan/graphics_errors.cpp"
        "src/graphics/vulkan/graphics_errors.hpp"
        "src/common_util/debug.hpp"
        "src/graphics/graphics_api.hpp"
        "src/application/window_interface.hpp"
        "src/application/window.hpp"
        "src/graphics/openGL/opengl_error.hpp"
        "src/graphics/openGL/opengl_error.cpp"
        "src/application/input_keys.hpp"
        "src/graphics/shader_interface.hpp"
        "src/application/imgui_handler.cpp"
        "src/application/imgui_handler.hpp"
        "src/graphics/openGL/opengl_shader.cpp"
        "src/graphics/openGL/opengl_shader.hpp"
        "src/game/ecs.hpp"
        "src/application/time.hpp"
        "src/game/entity_manager.hpp"
        "src/game/entity_manager.cpp"
        "src/application/statistics.cpp"
        "src/application/statistics.hpp"
        "src/application/platform/dialog.cpp"
        "src/application/platform/dialog.hpp"
        "src/graphics/renderer_interface.hpp"
        "src/graphics/model.hpp"
        "src/graphics/openGL/opengl_renderer.cpp"
        "src/graphics/openGL/opengl_renderer.hpp"
        "src/graphics/Camera.cpp"
        "src/graphics/Camera.hpp"
        "src/graphics/primitives.hpp"
        "src/game/transform.hpp"
        "src/application/input.cpp"
        "src/application/input.hpp"
        "src/application/misc.cpp"
        "src/application/misc.hpp"
        "src/common_util/misc.cpp"
        "src/common_util/misc.hpp"
        "src/events/signal.hpp"
        "src/graphics/light.hpp"
        "src/common_util/random.hpp"
        "src/game/camera_comp.hpp"
        "src/data/asset_manager.cpp"
        "src/data/asset_manager.hpp"
        "src/data/model_loader.cpp"
        "src/data/model_loader.hpp"
        "src/data/id_table.hpp"
        "src/graphics/renderer_structs.hpp"
        "src/game/ecs.cpp"
        "src/data/string.hpp"
        "src/graphics/framebuffer_interface.hpp"
        "src/graphics/openGL/gl_framebuffer.cpp"
        "src/graphics/openGL/gl_framebuffer.hpp"
        "src/graphics/openGL/gl_buffers.cpp"
        "src/graphics/openGL/gl_buffers.hpp"
        "src/data/hash_table.hpp"
        "src/graphics/util.hpp"
        "src/graphics/util.hpp"
        "src/graphics/util.hpp"
        "src/graphics/util.cpp"
        "src/graphics/image.hpp"
        "src/graphics/image.cpp"
        src/graphics/render_view.hpp
        src/graphics/openGL/shadow_map.hpp
        src/graphics/openGL/shadow_map.cpp
        src/common_util/os.cpp
        src/common_util/os.hpp
        src/graphics/openGL/gaussian_blur.cpp
        src/graphics/openGL/gaussian_blur.hpp
        src/application/platform/linux/linux_file_monitor.cpp
        src/application/platform/linux/linux_file_monitor.hpp
        src/application/platform/fs_monitor.hpp
        src/application/platform/fs_events.hpp
        src/application/platform/linux/linux_dialog.cpp
        src/application/platform/linux/linux_dialog.hpp
        src/application/platform/linux/linux_fs_monitor.cpp
        src/application/platform/linux/linux_fs_monitor.hpp
)

target_link_libraries(playgroundEngineLib glfw vulkan ${OPENGL_LIBRARIES} IMGUI glm assimp unordered_dense::unordered_dense)
