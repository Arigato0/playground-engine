#pragma once

#include "ecs.hpp"
#include "../application/engine.hpp"

void pge::__proxy_register_comp__(std::string_view name, IComponent *comp)
{
    Engine::entity_manager.register_prototype(name, comp);
}
