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

    // remove 'destroyed' entities
    EntityVec::iterator it = std::remove_if(m_entities.begin(), m_entities.end(), [](const std::shared_ptr<Entity> e){ return !e->isActive(); });
    m_entities.erase(it, m_entities.end());
    for (auto& p : m_entityMap)
    {
        EntityVec::iterator it = std::remove_if(p.second.begin(), p.second.end(), [](const std::shared_ptr<Entity> e){ return !e->isActive(); });
        p.second.erase(it, p.second.end());
    }
}

EntityVec& EntityManager::getEntities()
{
    return m_entities;
}

EntityVec& EntityManager::getEntities(const std::string& tag)
{
    return m_entityMap[tag];
}
