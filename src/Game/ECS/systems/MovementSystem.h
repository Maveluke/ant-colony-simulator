#pragma once
#include "../Components.h"

namespace MovementSystem {
  // Update entity position based on velocity
  // Returns true if entity bounced off a wall
  bool Update(CTransform& transform, float deltaTime,
    float worldWidth, float worldHeight);
}