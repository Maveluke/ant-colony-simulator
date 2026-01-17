#include "EntityManager.h"
#include "ErrorLog.h"
#include <iostream>

EntityManager::EntityManager() : m_entityPool(MAX_ENTITIES) {
  m_entitiesToAdd.reserve(MAX_ENTITIES);
}

void EntityManager::Update() {
  for (const auto& tag : m_entitiesToAdd) {
    if (!m_entityPool.AddEntity(tag)) {
      ErrorLog("MAX ENTITIES CAPACITY REACHED!");
      return;
    }
  }
  m_entitiesToAdd.clear();
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
  return m_entityPool.AddEntity(tag);
}

void EntityManager::AddEntityToQueue(const std::string& tag) {
  m_entitiesToAdd.push_back(tag);
}

bool EntityManager::DeleteEntity(Entity entity) {
  return m_entityPool.DeleteEntity(entity);
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