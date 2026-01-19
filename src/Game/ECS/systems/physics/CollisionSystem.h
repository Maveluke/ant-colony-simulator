#pragma once
#include "ECS/EntityManager.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "ECS/systems/EventBuffer.h"

namespace CollisionSystem {

  // Detect collisions and push typed events
  // Also handles spider pushout to prevent ants clipping into spider center
  void Update(EntityManager& em, SpatialGrid& grid, EventBuffer& events);

}