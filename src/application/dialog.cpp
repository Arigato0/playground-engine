#include "dialog.hpp"

#include <cstdio>
#include <string>

#include "fmt.hpp"
#include "log.hpp"
#include "../common_util/defer.hpp"

constexpr int MAX_PATH = 256;

std::optional<std::filesystem::path> linux_native_dialog(std::string_view start_dir)
{
    std::string_view gde = getenv("XDG_CURRENT_DESKTOP");

    std::string cmd_name;

    if (gde == "KDE")
    {
        cmd_name = fmt::format("kdialog --getopenfilename {}", start_dir);
    }
    else
    {
        cmd_name = "zenity --file-selection";
    }

    FILE *f = popen(cmd_name.c_str(), "r");

    if (f == nullptr)
    {
        return std::nullopt;
    }

    DEFER([&f]
    {
        pclose(f);
    });

    std::string buffer;

    buffer.reserve(MAX_PATH);

    char c = fgetc(f);

    while (c != '\n' && c != '\r')
    {
        buffer += c;
        c = fgetc(f);
    }

    return buffer;
}

std::optional<std::filesystem::path> pge::native_file_dialog(std::string_view start_dir)
{
#if defined(__linux__)
    return linux_native_dialog(start_dir);
#endif
}
