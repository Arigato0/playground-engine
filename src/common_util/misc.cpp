#pragma once

#include "misc.hpp"

#ifdef __has_include
#  if __has_include(<cxxabi.h>)
#    include <cxxabi.h>
#    define HAS_CXXABI
#  endif
#endif

#ifdef HAS_CXXABI
std::string_view pge::demangle_name(const char* mangled_name)
{
    int status = 0;
    size_t size = 0;
    return abi::__cxa_demangle(mangled_name, nullptr, &size, &status);
}
#else
std::string_view pge::demangle_name(const char* mangled_name)
{
   return mangled_name;
}
#endif
