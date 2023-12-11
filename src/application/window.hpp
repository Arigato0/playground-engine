#pragma once

#include "../common_util/macros.hpp"

#if PGE_SUPPORTED_PLATFORM
#define WINDOW_T pge::GlfwWindow
#else
#error "no window type has been implemented for this platform"
#endif