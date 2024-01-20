#pragma once

#include "linux_dialog.hpp"
#include "../../fmt.hpp"
#include "common_util/defer.hpp"
#include <cstdio>

std::optional<std::filesystem::path> pge::linux_native_dialog(std::string_view start_dir)
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

    buffer.reserve(256);

    char c = fgetc(f);

    while (c != '\n' && c != '\r')
    {
        buffer += c;
        c = fgetc(f);
    }

    return buffer;
}