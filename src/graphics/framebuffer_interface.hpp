#pragma once
#include <cstdint>

namespace pge
{
    class IFramebuffer
    {
    public:
        virtual ~IFramebuffer() = default;

        virtual uint32_t init() = 0;

        virtual void bind() = 0;
        virtual void unbind() = 0;

        virtual uint32_t get_texture() const = 0;
    };
}
