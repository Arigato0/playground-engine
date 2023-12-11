#pragma once

#include "../common_util/macros.hpp"

#if defined(__linux__) || defined(__WIN32__)
#include "GLFW/glfw3.h"
#endif

namespace pge
{
    static double program_time()
    {
#if PGE_SUPPORTED_PLATFORM
        return glfwGetTime();
#else
#error "a get time function has not been defined for this platform"
#endif
    }
}
