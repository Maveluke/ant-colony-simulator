#pragma once
#include "../EntityManager.h"
#include "SpatialGrid.h"
#include "EventBuffer.h"

namespace CollisionSystem {

  // Detect collisions and push typed events
  // Also handles spider pushout to prevent ants clipping into spider center
  void Update(EntityManager& em, SpatialGrid& grid, EventBuffer& events);

}