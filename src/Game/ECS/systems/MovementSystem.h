#pragma once
#include "../EntityManager.h"

namespace MovementSystem {
  // Update all entities with TRANSFORM component
  // Applies velocity to position and handles boundary bounce
  void Update(EntityManager& em, float deltaTime,
    float worldWidth, float worldHeight);
}