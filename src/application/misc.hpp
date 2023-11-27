#pragma once

#include <string>
#include <vector>

namespace pge
{
    struct VideoMode
    {
        int width;
        int height;
        int red_bits;
        int green_bits;
        int blue_bits;
        int refresh_rate;
    };

    std::vector<VideoMode> get_video_modes();
    std::vector<std::string> get_resolution_strings();
}
