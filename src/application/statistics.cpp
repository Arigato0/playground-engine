#include "statistics.hpp"

void pge::Statistics::calculate()
{
    auto current_time = program_time();

    m_delta_time = current_time - m_previous_time;
    ++m_frames;

    if (m_delta_time > 1.0)
    {
        m_fps = double(m_frames) / m_delta_time;
        m_frames = 0;
        m_previous_time = current_time;
    }
}
