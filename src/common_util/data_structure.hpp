#pragma once

#include <vector>

namespace util
{
    template<class T>
    static void concat(std::vector<T> &vector, T *data, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            vector.push_back(data[i]);
        }
    }

    template<class T, class O>
    static void concat(std::vector<T> &vector, O &other)
    {
        for (auto &item : other)
        {
            vector.push_back(std::move(item));
        }
    }
}
