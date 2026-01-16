#pragma once

// Entity Manager class
#include "EntityMemoryPool.h"

class EntityManager {
private:
	EntityMemoryPool m_entityPool;
	std::vector<std::string> m_entitiesToAdd; // Vector of tags of the corresponding entities

public:
	EntityManager();
	void Update();

	// CRUD Entity
	std::vector<Entity> GetEntities();
	std::vector<Entity> GetEntities(std::string tag);
	std::vector<Entity> GetEntitiesWithComponents(uint32_t componentMask);
	void AddEntity(const std::string& tag);
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
