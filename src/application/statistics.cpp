#include "statistics.hpp"

#include "engine.hpp"

void pge::Statistics::calculate()
{
    m_draw_calls = 0;
    m_vertices = 0;

    auto current_time = program_time();

    m_delta_time = current_time - m_previous_time;
    m_delta_time *= Engine::time_scale;
    m_previous_time = current_time;

    m_fps = 1 / m_delta_time;
}
