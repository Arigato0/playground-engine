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

		// if width and height are set to 0 should use framebuffer size
		virtual void blit(IFramebuffer *src, int width = 0, int height = 0, int attachment = 0, int x = 0, int y = 0) = 0;
		// same as blit but will blit all attachments
		virtual void blit_all_targets(IFramebuffer *src, int width = 0, int height = 0, int x = 0, int y = 0) = 0;

        [[nodiscard]]
		virtual uint32_t get_texture() const = 0;

        [[nodiscard]]
		virtual Image get_image() const = 0;
    };
}
