#pragma once

#include <string>
#include <cstdio>
#include <utility>
#include <string_view>
#include <sstream>

namespace fmt
{
    template<typename T>
    concept is_iterable = requires(T t)
    {
        t.begin();
        t.end();
    };

    template<is_iterable T>
    std::string to_string(const T& container);

    template<typename A, typename B>
    std::string to_string(const std::pair<A, B>& pair);

    inline std::string to_string(std::stringstream& ss)
    {
        return ss.str();
    }

    inline std::string to_string(char c)
    {
        return { c };
    }

    inline std::string to_string(std::nullptr_t ptr)
    {
        return "null";
    }

    inline std::string to_string(bool b)
    {
        return b ? "true" : "false";
    }

    template<class T>
    constexpr inline std::string string_of(const T& value)
    {
        // if not a number or a bool or a char convert it to a string
        if constexpr(std::is_arithmetic_v<T>
                && !std::is_same_v<T, bool>
                && !std::is_same_v<T, char>)
            return std::to_string(value);
        // if not null and can construct a string
        else if constexpr(
                std::is_constructible_v<std::string, T>
                && !std::is_same_v<T, std::nullptr_t>)
            return std::string{value};
        else
            return fmt::to_string(value);
    }

    template<class A, class B>
    inline std::string to_string(const std::pair<A, B>& pair)
    {
        return (string_of(pair.first) + ": " + string_of(pair.second));
    }

    template<is_iterable T>
    std::string to_string(const T& container)
    {
        if(container.empty())
            return "[]";

        std::string result;

        if(container.size() == 1)
        {
            result += "[ " + string_of(*container.begin()) + " ]";
            return result;
        }

        auto current = container.begin();

        result += "[ " + string_of(*current);

        current++;

        for(; current != container.end(); current++)
            result += ", " + string_of(*current);

        result += " ]";

        return result;
    }

    template<class... A>
    std::string format(std::string_view fmt, A&&... a)
    {
        if constexpr(sizeof...(a) == 0)
            return fmt.data();

        std::string buffer;

        ((buffer += string_of(a) + '\0'), ...);

        std::string output;
        size_t offset{};

        for(size_t i = 0; i < fmt.size(); i++)
        {
            if(fmt[i] == '{')
            {
                if(fmt[++i] == '{')
                    goto concat;

                for(char c = buffer[offset++]; c; c = buffer[offset++])
                {
                    output += c;
                }

                while(i < fmt.size() && fmt[i] != '}')
                    i++;
                continue;
            }

            concat:
            output += fmt[i];
        }

        return output;
    }

    template<class... A>
    inline int print(std::string_view fmt, A&&... a)
    {
        return std::printf(format(fmt, std::forward<A>(a)...).data());
    }

    template<class... A>
    inline int println(std::string_view fmt, A&&... a)
    {
        return std::puts(format(fmt, std::forward<A>(a)...).data());
    }

    template<typename... A>
    inline void fatal(std::string_view fmt, A&&... a)
    {
        std::printf(format(fmt, std::forward<A>(a)...).data());
        std::exit(-1);
    }
}