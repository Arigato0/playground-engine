#pragma once
#include <filesystem>
#include <optional>

namespace pge
{
    std::optional<std::filesystem::path> native_file_dialog(std::string_view start_dir);
}
