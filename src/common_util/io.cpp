#include <string>
#include <filesystem>
#include "io.hpp"

std::string util::read_file(const std::filesystem::path &path)
{
    auto file = fopen(path.c_str(), "r");

    if (file == nullptr)
    {
        return {};
    }

    auto size = std::filesystem::file_size(path);

    std::string output (size, '\0');

    fread(output.data(), sizeof(char), size, file);

    return output;
}