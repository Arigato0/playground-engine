#include "image.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

std::string_view pge::img_fmt_ext(ImgFmt format)
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

void pge::Image::save(std::string_view path, bool add_ext)
{
    stbi_flip_vertically_on_write(true);

    using enum ImgFmt;

    std::string filename;

    if (add_ext)
    {
        filename = get_filename(path);
    }
    else
    {
        filename = path;
    }

#define PARAMS filename.data(), width, height, channels, data.data()

    switch (format)
    {
        case Png:
        {
            stbi_write_png(PARAMS, width * channels);
            break;
        }
        case Jpg:
        {
            stbi_write_jpg(PARAMS, quality);
            break;
        }
        case Bmp:
        {
            stbi_write_bmp(PARAMS);
            break;
        }
    }

#undef PARAMS
}
