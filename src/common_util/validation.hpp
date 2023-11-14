#pragma once

#include "../application/error.hpp"

namespace util
{
    template<class T>
    inline static bool is_true(T *ptr)
    {
       return ptr != nullptr;
    }

    inline static bool is_true(bool b)
    {
        return b == true;
    }

    inline static bool is_true(pge::ErrorCode error)
    {
        return error == pge::ErrorCode::Ok;
    }
	
    template<class ...A>
    inline static bool any_true(A ...a)
    {
        bool all_true = false;

        ((is_true(a) && (all_true = true)), ...);

        return all_true;
    }
}
