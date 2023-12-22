#pragma once

#include <cstdint>
#include <string>
#include <string_view>

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

    static std::string_view img_fmt_ext(ImgFmt format)
    {
        using enum ImgFmt;

        switch (format)
        {
            case Png: return "png";
            case Jpg: return "jpg";
            case Bmp: return "bmp";
            case Hdr: return "hdr";
        }

        return "";
    }

    struct Image
    {
        int width;
        int height;
        int channels;
        ImgFmt format;
        // a pointer to format specific data. for jpg its just an int that specifies the jpg quality level
        void *other;
        std::vector<uint8_t> data;

        [[nodiscard]]
        std::string get_filename(std::string_view name) const
        {
            return fmt::format("{}.{}", name, img_fmt_ext(format));
        }
    };
}
