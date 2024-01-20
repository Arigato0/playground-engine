#include "dialog.hpp"

#include <cstdio>
#include <string>

#include "../fmt.hpp"
#include "../log.hpp"
#include "../../common_util/defer.hpp"
#include "linux/linux_dialog.hpp"


std::optional<std::filesystem::path> pge::native_file_dialog(std::string_view start_dir)
{
#if defined(__linux__)
    return linux_native_dialog(start_dir);
#endif
}
