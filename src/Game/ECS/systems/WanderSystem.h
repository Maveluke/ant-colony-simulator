#pragma once
#include "../EntityManager.h"

namespace WanderSystem {

  // Update wander direction and set velocity for all wandering entities
  // Works on any entity with TRANSFORM | WANDER | SPEED
  void Update(EntityManager& em, float deltaTime);

}