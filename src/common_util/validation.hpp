#pragma once

#include "../application/error.hpp"

namespace util
{
    template<class T>
    static bool is_true(T *ptr)
    {
       return ptr != nullptr;
    }

    static bool is_true(bool b)
    {
        return b == true;
    }

    static bool is_true(pge::ErrorCode error)
    {
        return error == pge::ErrorCode::Ok;
    }
	
    template<class ...A>
    static bool any_true(A ...a)
    {
        bool all_true = false;

        ((is_true(a) && (all_true = true)), ...);

        return all_true;
    }
}
