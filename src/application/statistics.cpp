#include "statistics.hpp"

void pge::Statistics::calculate()
{
    auto current_time = program_time();

    m_delta_time = current_time - m_previous_time;
    m_previous_time = current_time;

    m_fps = 1 / m_delta_time;
}
