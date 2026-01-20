#include "EntityManager.h"
#include "ErrorLog.h"
#include <iostream>

EntityManager::EntityManager() : m_entityPool(MAX_ENTITIES) {
  m_entitiesToAdd.reserve(MAX_ENTITIES);

  // Pre-allocate cache vectors
  m_cachedAnts.reserve(2000);
  m_cachedAntsWithCollider.reserve(2000);
  m_cachedFoods.reserve(500);
  m_cachedColonies.reserve(10);
  m_cachedSpiders.reserve(50);
  m_cachedDraggables.reserve(500);
  m_cachedTransforms.reserve(3000);
}

void EntityManager::Update() {
  bool addedEntities = !m_entitiesToAdd.empty();

  for (const auto& tag : m_entitiesToAdd) {
    if (!m_entityPool.AddEntity(tag)) {
      ErrorLog("MAX ENTITIES CAPACITY REACHED!");
      return;
    }
  }
  m_entitiesToAdd.clear();

  // Mark caches dirty if we added entities
  if (addedEntities) {
    m_cachesDirty = true;
  }
}

// === CACHE MANAGEMENT ===

void EntityManager::RebuildCaches() {
  if (!m_cachesDirty) return;

  // Clear all caches
  m_cachedAnts.clear();
  m_cachedAntsWithCollider.clear();
  m_cachedFoods.clear();
  m_cachedColonies.clear();
  m_cachedSpiders.clear();
  m_cachedDraggables.clear();
  m_cachedTransforms.clear();

  // Single pass through all entities to build all caches
  for (Entity i = 1; i < MAX_ENTITIES + 1; i++) {
    if (!m_entityPool.m_active[i]) continue;

    uint32_t mask = m_entityPool.m_componentBitmask[i];

    // Check each cache's requirements
    if ((mask & TRANSFORM) == TRANSFORM) {
      m_cachedTransforms.push_back(i);
    }

    if ((mask & MASK_ANT) == MASK_ANT) {
      m_cachedAnts.push_back(i);
    }

    if ((mask & MASK_ANT_COLLIDER) == MASK_ANT_COLLIDER) {
      m_cachedAntsWithCollider.push_back(i);
    }

    if ((mask & MASK_FOOD) == MASK_FOOD) {
      m_cachedFoods.push_back(i);
    }

    if ((mask & MASK_COLONY) == MASK_COLONY) {
      m_cachedColonies.push_back(i);
    }

    if ((mask & MASK_SPIDER) == MASK_SPIDER) {
      m_cachedSpiders.push_back(i);
    }

    if ((mask & MASK_DRAGGABLE) == MASK_DRAGGABLE) {
      m_cachedDraggables.push_back(i);
    }
  }

  m_cachesDirty = false;
}

const std::vector<Entity>& EntityManager::GetAnts() {
  if (m_cachesDirty) RebuildCaches();
  return m_cachedAnts;
}

const std::vector<Entity>& EntityManager::GetAntsWithCollider() {
  if (m_cachesDirty) RebuildCaches();
  return m_cachedAntsWithCollider;
}

const std::vector<Entity>& EntityManager::GetFoods() {
  if (m_cachesDirty) RebuildCaches();
  return m_cachedFoods;
}

const std::vector<Entity>& EntityManager::GetColonies() {
  if (m_cachesDirty) RebuildCaches();
  return m_cachedColonies;
}

const std::vector<Entity>& EntityManager::GetSpiders() {
  if (m_cachesDirty) RebuildCaches();
  return m_cachedSpiders;
}

const std::vector<Entity>& EntityManager::GetDraggables() {
  if (m_cachesDirty) RebuildCaches();
  return m_cachedDraggables;
}

const std::vector<Entity>& EntityManager::GetTransformEntities() {
  if (m_cachesDirty) RebuildCaches();
  return m_cachedTransforms;
}

// CRUD Entity
std::vector<Entity> EntityManager::GetEntities() {
  return m_entityPool.GetEntities();
}

std::vector<Entity> EntityManager::GetEntities(std::string tag) {
  return m_entityPool.GetEntities(tag);
}

std::vector<Entity> EntityManager::GetEntitiesWithComponents(uint32_t componentMask) {
  return m_entityPool.GetEntitiesWithComponents(componentMask);
}

Entity EntityManager::AddEntityImmediate(const std::string& tag) {
  Entity e = m_entityPool.AddEntity(tag);
  if (e != INVALID_ENTITY) {
    m_cachesDirty = true;
  }
  return e;
}

void EntityManager::AddEntityToQueue(const std::string& tag) {
  m_entitiesToAdd.push_back(tag);
}

bool EntityManager::DeleteEntity(Entity entity) {
  bool deleted = m_entityPool.DeleteEntity(entity);
  if (deleted) {
    m_cachesDirty = true;
  }
  return deleted;
}

// CRUD Component
bool EntityManager::HasComponents(uint32_t componentMask, Entity entity) {
  return m_entityPool.HasComponents(componentMask, entity);
}

bool EntityManager::AddComponent(ComponentType componentType, Entity entity) {
  return m_entityPool.AddComponent(componentType, entity);
}

bool EntityManager::RemoveComponent(ComponentType componentType, Entity entity) {
  return m_entityPool.RemoveComponent(componentType, entity);
}

std::string EntityManager::GetTag(Entity entity) {
  return m_entityPool.GetTag(entity);
}