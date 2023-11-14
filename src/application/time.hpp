#pragma once

#if defined(__linux__) || defined(__WIN32__)
#include "GLFW/glfw3.h"
#endif

namespace pge
{
    double program_time()
    {
#if defined(__linux__) || defined(__WIN32__)
        return glfwGetTime();
#else
#error "a get time function has not been defined for this platform"
#endif
    }
}
