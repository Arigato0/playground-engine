#include "misc.hpp"

#include "fmt.hpp"
#include "GLFW/glfw3.h"

std::vector<pge::VideoMode> pge::get_video_modes()
{
    int count;

    auto *monitor = glfwGetPrimaryMonitor();

    auto *vid_modes = glfwGetVideoModes(monitor, &count);

    std::vector<VideoMode> modes;

    modes.reserve(count);

    for (int i = 0; i < count; i++)
    {
        auto mode = vid_modes[i];

        modes.emplace_back(
            VideoMode
            {
                .width          = mode.width,
                .height         = mode.height,
                .red_bits       = mode.redBits,
                .green_bits     = mode.greenBits,
                .blue_bits      = mode.blueBits,
                .refresh_rate   = mode.refreshRate
            });
    }

    return modes;
}

std::vector<std::string> pge::get_resolution_strings()
{
    auto video_modes = get_video_modes();

    std::vector<std::string> output;

    output.reserve(video_modes.size());

    for (auto mode : video_modes)
    {
        auto str = fmt::format("{}x{} ({})", mode.width, mode.height, mode.refresh_rate);

        output.push_back(std::move(str));
    }

    return output;
}
