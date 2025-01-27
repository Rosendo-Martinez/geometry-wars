#include "Entity.h"

bool Entity::isActive() const
{
    return m_active;
}

const size_t Entity::id() const
{
    return m_id;
}

Entity::Entity(const std::string& tag, const size_t id)
    : m_tag(tag), m_id(id) {}

const std::string & Entity::tag() const 
{ 
    return m_tag; 
}

void Entity::destroy() 
{ 
    m_active = false; 
}