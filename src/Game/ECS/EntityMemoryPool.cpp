#include "EntityMemoryPool.h"

EntityMemoryPool::EntityMemoryPool(size_t max_entities) {
  std::apply([max_entities](auto&... vectors) {
    ((vectors.resize(max_entities)), ...);
    }, m_pool);

  // Leave Entity 0 unused
  m_active.resize(max_entities + 1, false);
  m_tags.resize(max_entities + 1, "");
  m_componentBitmask.resize(max_entities + 1, 0);
  m_freeList.reserve(max_entities);
  for (size_t i = max_entities; i > 0; i--) {
    m_freeList.push_back(i);
  }
}

// Helper functions: 
size_t EntityMemoryPool::GetNumEntities() const {
  return m_numEntities;
}

// CRUD Entity
Entity EntityMemoryPool::AddEntity(const std::string& tag) {
  if (m_freeList.empty()) return 0;

  Entity i = m_freeList.back();
  m_freeList.pop_back();

  if (i >= m_active.size()) return 0;

  // Initialize entity data
  m_componentBitmask[i] = 0;
  m_active[i] = true;
  m_tags[i] = tag;
  m_numEntities++;

  return i;
}

std::vector<Entity> EntityMemoryPool::GetEntities() {
  std::vector<Entity> ret;
  ret.reserve(m_numEntities);
  for (Entity i = 1; i < m_active.size(); i++) {
    if (m_active[i]) ret.push_back(i);
  }
  return ret;
}

std::vector<Entity> EntityMemoryPool::GetEntities(const std::string& tag) {
  std::vector<Entity> ret;
  ret.reserve(m_numEntities);
  for (Entity i = 1; i < m_active.size(); i++) {
    if (m_active[i] && m_tags[i] == tag) ret.push_back(i);
  }
  return ret;
}

std::vector<Entity> EntityMemoryPool::GetEntitiesWithComponents(uint32_t componentMask) {
  std::vector<Entity> ret;
  ret.reserve(m_numEntities);
  for (Entity i = 1; i < m_active.size(); i++) {
    if (m_active[i] && (m_componentBitmask[i] & componentMask) == componentMask) {
      ret.push_back(i);
    }
  }
  return ret;
}

bool EntityMemoryPool::DeleteEntity(Entity entity) {
  if (!m_active[entity]) return false;
  m_active[entity] = false;
  m_freeList.push_back(entity);
  m_numEntities--;
  return true;
}

// Component CRUD
bool EntityMemoryPool::HasComponents(uint32_t componentMask, Entity entity) {
  return (m_componentBitmask[entity] & componentMask) == componentMask;
}

bool EntityMemoryPool::AddComponent(ComponentType componentType, Entity entity) {
  if (HasComponents(componentType, entity)) return false;
  m_componentBitmask[entity] |= componentType;
  return true;
}

bool EntityMemoryPool::RemoveComponent(ComponentType componentType, Entity entity) {
  if (!HasComponents(componentType, entity)) return false;
  m_componentBitmask[entity] &= ~componentType;
  return true;
}

const std::string& EntityMemoryPool::GetTag(Entity entity) {
  return m_tags[entity];
}