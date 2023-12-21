#include "gl_buffers.hpp"

pge::GlBufferBuilder& pge::GlBufferBuilder::start()
{
    glGenVertexArrays(1, &m_buffer.vao);
    glBindVertexArray(m_buffer.vao);

    return *this;
}

pge::GlBufferBuilder& pge::GlBufferBuilder::vbo(const std::vector<Vertex>& verts)
{
    set_vbo(verts.data(), sizeof(Vertex) * verts.size());
    return *this;
}

pge::GlBufferBuilder& pge::GlBufferBuilder::vbo(const std::span<float>& verts)
{
    set_vbo(verts.data(), sizeof(float) * verts.size());
    return *this;
}

pge::GlBufferBuilder& pge::GlBufferBuilder::ebo(const std::vector<uint32_t>& verts)
{
    glGenBuffers(1, &m_buffer.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer.ebo);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * verts.size(), verts.data(), GL_STATIC_DRAW);

    return *this;
}

pge::GlBufferBuilder& pge::GlBufferBuilder::stride(uint32_t value)
{
    m_stride = value;

    return *this;
}

pge::GlBufferBuilder& pge::GlBufferBuilder::attr(uint32_t size, uint32_t offset)
{
    assert(m_stride != UINT32_MAX && "stride has not been set");

    auto layout = m_layout++;

    glVertexAttribPointer(layout, size, GL_FLOAT, GL_FALSE, m_stride, (void*)offset);
    glEnableVertexAttribArray(layout);

    return *this;
}

pge::GlBuffers pge::GlBufferBuilder::finish() const
{
    glBindVertexArray(0);
    return m_buffer;
}

void pge::GlBufferBuilder::set_vbo(const void* data, size_t size)
{
    glGenBuffers(1, &m_buffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
