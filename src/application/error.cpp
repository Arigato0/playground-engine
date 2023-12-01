#include "error.hpp"

constexpr std::array ERROR_MESSAGES
{
        "No error",
        "Could not open a window",
        "The graphics library could not initialize",
        "An error occurred while drawing",
        "The engine has not been initialized"
};

std::string_view pge::error_message(ErrorCode type)
{
    auto idx = (int)type;

    if (idx >= ERROR_MESSAGES.size())
    {
        return {};
    }

    return ERROR_MESSAGES[idx];
}


