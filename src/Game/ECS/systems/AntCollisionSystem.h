#pragma once
#include "ECS/EntityManager.h"
#include "ECS/systems/EventBuffer.h"

namespace AntCollisionSystem {

  // Handle all ant-related collision events:
  // - Ant <-> Food: Start dragging if ant is wandering/following trail
  // - Food <-> Colony: Deposit food, release draggers
  // - Ant <-> Spider: Combat damage, state transitions
  void HandleCollisions(EntityManager& em, EventBuffer& events);
}