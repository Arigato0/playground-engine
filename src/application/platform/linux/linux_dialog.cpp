#pragma once

#include "linux_dialog.hpp"
#include "../../fmt.hpp"
#include "common_util/defer.hpp"
#include <cstdio>
#include <sys/poll.h>
#include <sys/ioctl.h>

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

    auto *f = popen(cmd_name.c_str(), "r");

    if (f == nullptr)
    {
        return std::nullopt;
    }

    DEFER([&f]
    {
        pclose(f);
    });

	pollfd pfd
	{
		.fd = f->_fileno,
		.events = POLLIN
	};

	auto result = poll(&pfd, 1, 0);

	if (result < 0)
	{
		return std::nullopt;
	}

	uint32_t bytes_to_read;

	ioctl(f->_fileno, FIONREAD, &bytes_to_read);

    std::string buffer;

    buffer.reserve(bytes_to_read);

    char c = fgetc(f);

    while (c != '\n' && c != '\r')
    {
        buffer += c;
        c = fgetc(f);
    }

    return buffer;
}