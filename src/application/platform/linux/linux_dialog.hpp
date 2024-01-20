#pragma once

#include <optional>
#include <filesystem>
#include <string_view>

namespace pge
{
	std::optional<std::filesystem::path> linux_native_dialog(std::string_view start_dir);
}
