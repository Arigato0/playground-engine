#pragma once

#if defined(__linux__) || defined(WIN32)
#define WINDOW_T pge::GlfwWindow
#else
#error "no window type has been implemented for this platform"
#endif