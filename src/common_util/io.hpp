#pragma once

#include <string>
#include <filesystem>

namespace util
{
    std::string read_file(const std::filesystem::path &path);
}

