#pragma once

#include "Components.h"
#include "Definition.h"
#include <tuple>
#include <vector>
#include <string>

constexpr uint32_t MAX_ENTITIES{ 10 }; // TODO: set to original value 50

using EntityComponentVectorTuple =
std::tuple <
	std::vector<CPlayer>,
	std::vector<CTransform>,
	std::vector<CAABB>,
	std::vector<CQuadRenderer>,
	std::vector<CCircleRenderer>>;

// Cache-friendly memory pool for Entity Manager

class EntityMemoryPool
{
private:
	friend class EntityManager;

	size_t m_numEntities = 0;
	EntityComponentVectorTuple m_pool;
	std::vector<bool> m_active;
	std::vector<std::string> m_tags;
	std::vector<uint32_t> m_componentBitmask;

	EntityMemoryPool(size_t max_entities);

	// Helper functions: 
	size_t GetNumEntities() const;
	size_t GetNextIndexEntity(size_t start_i = 0);

	// CRUD Entity
	bool AddEntity(const std::string& tag);
	std::vector<Entity> GetEntities();
	std::vector<Entity> GetEntities(const std::string& tag);
	std::vector<Entity> GetEntitiesWithComponents(uint32_t componentMask);
	bool DeleteEntity(Entity entity);

	// Component CRUD
	bool HasComponent(ComponentType componentType, Entity entity);
	bool AddComponent(ComponentType componentType, Entity entity);
	bool RemoveComponent(ComponentType componentType, Entity entity);

	template <typename T>
	T& GetComponent(Entity entity) {
		return std::get<std::vector<T>>(m_pool)[entity];
	}


	const std::string& GetTag(Entity entity);
};
