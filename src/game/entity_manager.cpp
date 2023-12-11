#include "entity_manager.hpp"

void pge::EntityManager::start()
{
    for (auto &[_, entity] : m_entities)
    {
        entity.start_components();
    }
}

void pge::EntityManager::update(double delta_time)
{
    for (auto &[_, entity] : m_entities)
    {
        entity.update_components(delta_time);
    }
}

pge::Entity* pge::EntityManager::find(std::string_view name)
{
    auto iter = m_entities.find(name);

    if (iter == m_entities.end())
    {
        return nullptr;
    }

    return &iter->second;
}
