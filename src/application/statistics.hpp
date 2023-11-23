#pragma once
#include <cstdint>

#include "time.hpp"

namespace pge
{

    struct FrameStats
    {
        double delta_time;
        uint32_t fps;
        uint32_t draw_calls;
        uint32_t vertices;
    };

    class Statistics
    {
    public:

        Statistics() = default;

        void calculate();

        [[nodiscard]]
        uint32_t fps() const
        {
            return m_fps;
        }

        [[nodiscard]]
        uint32_t draw_calls() const
        {
            return m_draw_calls;
        }

         [[nodiscard]]
        uint32_t vertices() const
        {
            return m_vertices;
        }

        [[nodiscard]]
        double delta_time() const
        {
            return m_delta_time;
        }

        [[nodiscard]]
        FrameStats stats() const
        {
            return
            {
                .delta_time = m_delta_time,
                .fps = m_fps,
                .draw_calls = m_draw_calls,
                .vertices = m_vertices,
            };
        }

        void report_draw_call(uint32_t amount = 1)
        {
            m_draw_calls += amount;
        }

        void report_verticies(uint32_t amount)
        {
            m_vertices += amount;
        }

    private:
        double   m_previous_time = program_time();
        double	 m_delta_time;
        uint32_t m_fps;
        uint32_t m_draw_calls;
        uint32_t m_vertices;
    };
}
