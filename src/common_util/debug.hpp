#pragma once

#include <string>
#include <source_location>
#include "../application/fmt.hpp"

namespace util
{
    static inline std::string stacktrace_string(std::string_view message,
        std::source_location location = std::source_location::current())
    {
        return fmt::format("{}: ({}:{}) {}",
        location.file_name(),
        location.line(),
        location.column(),
        message);
    }
}