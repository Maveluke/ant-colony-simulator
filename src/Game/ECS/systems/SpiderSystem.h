#pragma once
#include "ECS/EntityManager.h"
#include "ECS/systems/grids/SpatialGrid.h"

namespace SpiderSystem {

  // Update all spider AI - hunt nearest ant, wander if none nearby
  void Update(EntityManager& em, SpatialGrid& grid, float deltaTime);

}