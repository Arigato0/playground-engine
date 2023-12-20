#pragma once
#include "../framebuffer_interface.hpp"
#include "../../application/window_interface.hpp"

namespace pge
{
    class GlFramebuffer : public IFramebuffer
    {
    public:
        uint32_t init() override;

        ~GlFramebuffer() override;

        uint32_t get_texture() const override;

        void bind() override;

        void unbind() override;

    private:
        uint32_t m_fbo;
        uint32_t m_rbo;
        uint32_t m_texture;

        void on_resize(IWindow*, int width, int height) const;
    };
}
