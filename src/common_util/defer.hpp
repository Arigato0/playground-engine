#pragma once

#include "constraints.hpp"
#include "macros.hpp"

#define DEFER(exp) util::Defer CONCAT(__DEFER__, __COUNTER__) ((exp))

namespace util
{
    template<IsFunction FN>
    class Defer
    {
    public:
        Defer(FN fn) :
            m_fn(fn)
        {}

        ~Defer()
        {
            m_fn();
        }

    private:
        FN m_fn;
    };
}

