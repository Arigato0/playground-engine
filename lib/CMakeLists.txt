cmake_minimum_required(VERSION 3.22)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw-3.3.8)

add_library(IMGUI STATIC)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui-docking)
target_sources( IMGUI
                PRIVATE
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui.cpp
                    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
                )

target_include_directories( IMGUI
                            PUBLIC ${IMGUI_DIR}
                            PUBLIC ${IMGUI_DIR}/backends
                            PUBLIC ${SDL3_DIR}/include
                            )
target_link_libraries(IMGUI glfw vulkan ${OPENGL_LIBRARIES})