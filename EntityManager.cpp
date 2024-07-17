#include "EntityManager.h"
#include <algorithm>

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
    auto e = std::shared_ptr<Entity>(new Entity(tag, m_totalEntities++));
    m_toAdd.push_back(e);
    return e;
}

void EntityManager::update()
{
    // add entities on wait list
    for (auto e : m_toAdd)
    {
        m_entities.push_back(e);
        m_entityMap[e->tag()].push_back(e);
    }
    m_toAdd.clear();

    // remove destroyed bullets
    const auto newEndEntities = std::remove_if(m_entities.begin(), m_entities.end(), [](const std::shared_ptr<Entity> e){ return !e->isActive(); });
    m_entities.erase(newEndEntities, m_entities.end());
    const auto newEndBullets = std::remove_if(m_entityMap["bullet"].begin(), m_entityMap["bullet"].end(), [](const std::shared_ptr<Entity> e){ return !e->isActive(); });
    m_entityMap["bullet"].erase(newEndBullets, m_entityMap["bullet"].end());

}

EntityVec& EntityManager::getEntities()
{
    return m_entities;
}

EntityVec& EntityManager::getEntities(const std::string& tag)
{
    return m_entityMap[tag];
}
