#pragma once
#include <cstdint>
#include <vector>

#include "image.hpp"

namespace pge
{
    class IFramebuffer
    {
    public:
        virtual ~IFramebuffer() = default;

        virtual uint32_t init(int samples) = 0;

        virtual void bind() = 0;
        virtual void unbind() = 0;

		virtual void set_samples(int n) = 0;

        virtual uint32_t get_texture() const = 0;
        virtual Image get_image() const = 0;
    };
}
