#pragma once

#include <string>
#include <memory>
#include "Components.h"

class Entity
{
public:
    std::shared_ptr<CTransform> cTransform;
    std::shared_ptr<CShape>     cShape;
    std::shared_ptr<CCollision> cCollision;
    std::shared_ptr<CInput>     cInput;
    std::shared_ptr<CLifespan>  cLifespan;
    std::shared_ptr<CScore>     cScore;

    bool isActive() const;
    const size_t id() const;
    const std::string & tag() const;
    void destroy();

private:
    friend class EntityManager;

    std::string m_tag    = "default";
    bool        m_active = true;
    size_t      m_id     = 0;

    Entity(const std::string& tag, const size_t id);
};