#pragma once

// Entity Manager class
#include "EntityMemoryPool.h"
#include "Components.h"

// Pre-defined component masks for common entity types (for caching)
constexpr uint32_t MASK_ANT = ANT | TRANSFORM | WANDER | SPEED;
constexpr uint32_t MASK_ANT_COLLIDER = ANT | TRANSFORM | CIRCLE_COLLIDER;
constexpr uint32_t MASK_FOOD = FOOD | TRANSFORM | CIRCLE_COLLIDER;
constexpr uint32_t MASK_COLONY = COLONY | TRANSFORM;
constexpr uint32_t MASK_SPIDER = SPIDER | TRANSFORM | CIRCLE_COLLIDER;
constexpr uint32_t MASK_DRAGGABLE = DRAGGABLE | TRANSFORM;

class EntityManager {
private:
  EntityMemoryPool m_entityPool;
  std::vector<std::string> m_entitiesToAdd; // Vector of tags of the corresponding entities

  // === CACHED ENTITY LISTS ===
  // These are rebuilt only when entities are added/removed
  std::vector<Entity> m_cachedAnts;
  std::vector<Entity> m_cachedAntsWithCollider;
  std::vector<Entity> m_cachedFoods;
  std::vector<Entity> m_cachedColonies;
  std::vector<Entity> m_cachedSpiders;
  std::vector<Entity> m_cachedDraggables;
  std::vector<Entity> m_cachedTransforms;  // All entities with TRANSFORM (for spatial grid)
  bool m_cachesDirty = true;  // Set to true when entities added/removed

  void RebuildCaches();

public:
  EntityManager();
  void Update();

  // Mark caches as dirty (called internally on add/delete)
  void InvalidateCaches() { m_cachesDirty = true; }

  // === CACHED GETTERS (use these for hot paths!) ===
  const std::vector<Entity>& GetAnts();
  const std::vector<Entity>& GetAntsWithCollider();
  const std::vector<Entity>& GetFoods();
  const std::vector<Entity>& GetColonies();
  const std::vector<Entity>& GetSpiders();
  const std::vector<Entity>& GetDraggables();
  const std::vector<Entity>& GetTransformEntities();

  // CRUD Entity
  std::vector<Entity> GetEntities();
  std::vector<Entity> GetEntities(std::string tag);
  std::vector<Entity> GetEntitiesWithComponents(uint32_t componentMask);  // Slow - avoid in hot paths
  Entity AddEntityImmediate(const std::string& tag);
  void AddEntityToQueue(const std::string& tag);
  bool DeleteEntity(Entity entity);

  //CRUD Component
  bool HasComponents(uint32_t componentMask, Entity entity);
  bool AddComponent(ComponentType componentType, Entity entity);
  bool RemoveComponent(ComponentType componentType, Entity entity);

  template <typename T>
  T& GetComponent(Entity entity) {
    return m_entityPool.GetComponent<T>(entity);
  }

  std::string GetTag(Entity entity);
};
