#pragma once
#include "ECS/EntityManager.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "ECS/systems/grids/PheromoneGrid.h"
#include "ECS/systems/EventBuffer.h"
#include "ECS/systems/physics/DragSystem.h"

namespace AntSystem {

  // Update all ant AI 
  void Update(EntityManager& em, SpatialGrid& grid, PheromoneGrid& pheromones,
    float deltaTime);

  // Handle collision events - food pickup, colony deposit, spider encounter
  void HandleCollisions(EntityManager& em, EventBuffer& events);
}