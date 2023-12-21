#pragma once

#include <vector>
#include <glad/glad.h>

#include "../model.hpp"

namespace pge
{
    struct GlBuffers
    {
        GLuint vbo;
        GLuint vao;
        GLuint ebo;

        void free() const
        {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
        }
    };

    class GlBufferBuilder
    {
    public:
        GlBufferBuilder& start();

        GlBufferBuilder& vbo(const std::vector<Vertex> &verts);

        GlBufferBuilder& vbo(const std::span<float> &verts);

        GlBufferBuilder& ebo(const std::vector<uint32_t> &verts);

        GlBufferBuilder& stride(uint32_t value);

        GlBufferBuilder& attr(uint32_t size, uint32_t offset);

        [[nodiscard]]
        GlBuffers finish() const;

    private:
        GlBuffers m_buffer {};
        int m_layout = 0;
        uint32_t m_stride = UINT32_MAX;

        void set_vbo(const void *data, size_t size);
    };
}
