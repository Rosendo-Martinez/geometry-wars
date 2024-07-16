#include <vector>
#include <memory>
#include "Entity.h"

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;

class EntityManager
{
    EntityVec m_entities;
    EntityVec m_toAdd;
    EntityMap m_entityMap;
    size_t    m_totalEntities = 0;

public:
    EntityManager() {}
    void update(); // removes dead entities & adds entities in wait list, should be called at begging of next frame (delayed affect)
    std::shared_ptr<Entity> addEntity(const std::string& tag);
    EntityVec& getEntities();
    EntityVec& getEntities(const std::string& tag);
};

// creating entities
// auto e = std::shared_ptr<Entity>(new Entity(args));