#include "util.hpp"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void pge::screen_shot(std::string_view path, const Image &image)
{
    stbi_flip_vertically_on_write(true);

    using enum ImgFmt;

    switch (image.format)
    {
        case Png:
        {
            stbi_write_png(path.data(), image.width, image.height, image.channels, image.data.data(), image.width * image.channels);
            break;
        }
        case Jpg:
        {
            stbi_write_jpg(path.data(), image.width, image.height, image.channels, image.data.data(), intptr_t(image.other));
            break;
        }
        case Bmp:
        {
            stbi_write_bmp(path.data(), image.width, image.height, image.channels, image.data.data());
            break;
        }
    }
}
