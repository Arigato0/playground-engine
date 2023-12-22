#pragma once
#include "../framebuffer_interface.hpp"
#include "../../application/engine.hpp"
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

        Image get_image() const override;

    private:
        uint32_t m_fbo;
        uint32_t m_rbo;
        uint32_t m_texture;
        Connection<void, IWindow*, int, int> *m_on_resize_con;

        void on_resize(IWindow*, int width, int height) const;
    };
}
