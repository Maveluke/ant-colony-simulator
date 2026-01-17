#pragma once

#include "Components.h"
#include "Definition.h"
#include <tuple>
#include <vector>
#include <string>

constexpr uint32_t MAX_ENTITIES{ 5000 };

using EntityComponentVectorTuple =
std::tuple <
  std::vector<CTransform>,
  std::vector<CAABB>,
  std::vector<CHealth>,
  std::vector<CCombat>,
  std::vector<CAnt>,
  std::vector<CFood>,
  std::vector<CColony>,
  std::vector<CSpider>,
  std::vector<CQuadRenderer>,
  std::vector<CCircleRenderer>>;

// Cache-friendly memory pool for Entity Manager

class EntityMemoryPool {
private:
  friend class EntityManager;

  size_t m_numEntities = 0;
  // In the current implementation, we make sure that Entity 0 is not used
  EntityComponentVectorTuple m_pool;
  std::vector<Entity> m_freeList;
  std::vector<bool> m_active;
  std::vector<std::string> m_tags;
  std::vector<uint32_t> m_componentBitmask;

  EntityMemoryPool(size_t max_entities);

  // Helper functions: 
  size_t GetNumEntities() const;

  // CRUD Entity
  Entity AddEntity(const std::string& tag);
  std::vector<Entity> GetEntities();
  std::vector<Entity> GetEntities(const std::string& tag);
  std::vector<Entity> GetEntitiesWithComponents(uint32_t componentMask);
  bool DeleteEntity(Entity entity);

  // Component CRUD
  bool HasComponents(uint32_t componentMask, Entity entity);
  bool AddComponent(ComponentType componentType, Entity entity);
  bool RemoveComponent(ComponentType componentType, Entity entity);

  template <typename T>
  T& GetComponent(Entity entity) {
    return std::get<std::vector<T>>(m_pool)[entity];
  }


  const std::string& GetTag(Entity entity);
};
