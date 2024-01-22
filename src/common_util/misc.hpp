#pragma once
#include <string>
#include <typeinfo>

#include "data/hash_table.hpp"

#define ENABLE_TRANSPARENT_HASH string_hash, std::equal_to<>
namespace pge
{
    struct string_hash
    {
        using hash_type = Hash<std::string_view>;

        using is_transparent = void;
		using is_avalanching = void;

        size_t operator()(const char* str) const { return hash_type{}(str); }
        size_t operator()(std::string_view str) const { return hash_type{}(str); }
        size_t operator()(std::string const& str) const { return hash_type{}(str); }
    };

    std::string_view demangle_name(const char *mangled_name);

    template<class T>
    static std::string_view type_name()
    {
        return demangle_name(typeid(T).name());
    }

    template<class ...A>
    struct overload : A... { using A::operator()...; };

    template<class... A>
    overload(A...) -> overload<A...>;
}
