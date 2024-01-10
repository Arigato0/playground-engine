#pragma once
#include <cstdint>

#include "time.hpp"

namespace pge
{

    struct FrameStats
    {
        double delta_time;
        uint32_t fps;
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
            };
        }

    private:
        double   m_previous_time = program_time();
        double	 m_delta_time;
        uint32_t m_fps;
    };
}
