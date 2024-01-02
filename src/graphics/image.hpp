#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "../application/fmt.hpp"

namespace pge
{
    enum class ImgFmt : uint8_t
    {
        Png,
        Jpg,
        Bmp,
        Hdr
    };

    static std::string_view img_fmt_ext(ImgFmt format);

    // image container for png, jpg, and bmp
    struct Image
    {
        int width;
        int height;
        int channels;
        ImgFmt format;
        // image quality only relevent to jpg
        int quality;
        std::vector<uint8_t> data;

        [[nodiscard]]
        std::string get_filename(std::string_view name) const
        {
            return fmt::format("{}.{}", name, img_fmt_ext(format));
        }

        void save(std::string_view path, bool add_ext = true);
    };


}
