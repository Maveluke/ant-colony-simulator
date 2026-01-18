#pragma once
#include "../EntityManager.h"
#include "grids/SpatialGrid.h"
#include "grids/PheromoneGrid.h"
#include "EventBuffer.h"
#include "physics/DragSystem.h"

namespace AntSystem {

  // Update all ant AI 
  void Update(EntityManager& em, SpatialGrid& grid, PheromoneGrid& pheromones,
    float deltaTime);

  // Handle collision events - food pickup, colony deposit, spider encounter
  void HandleCollisions(EntityManager& em, EventBuffer& events);
}