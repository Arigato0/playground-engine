#pragma once
#include <cstdint>
#include <vector>

#include "image.hpp"

namespace pge
{
	// the interface for interacting with a renderer owned framebuffer
    class IFramebuffer
    {
    public:
        virtual ~IFramebuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual void set_samples(int n) = 0;

        [[nodiscard]]
		virtual uint32_t get_texture() const = 0;

        [[nodiscard]]
		virtual Image get_image() const = 0;
    };
}
