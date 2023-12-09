#pragma once

#include <cstdlib>

namespace pge
{
    template<class T>
    static T rand_range(T min, T max)
    {
        return rand() % (max + T(1) - min) + min;
    }

    float rand_float(float min, float max)
    {
        return min + ((float)(rand()) / RAND_MAX * (max - min));
    }

    static glm::vec3 rand_vec3(float min, float max)
    {
        return {rand_float(min, max), rand_float(min, max),rand_float(min, max)};
    }
}