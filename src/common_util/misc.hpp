#pragma once
#include <string>
#include <typeinfo>

namespace pge
{
    std::string_view demangle_name(const char *mangled_name);

    template<class T>
    static std::string_view type_name()
    {
        return demangle_name(typeid(T).name());
    }
}
