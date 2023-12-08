#pragma once

#include <cstdlib>

namespace pge
{
    template<class T>
    T rand_range(T min, T max)
    {
        return rand() % (max + 1 - min) + min;
    }
}