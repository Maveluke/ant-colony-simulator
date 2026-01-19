#pragma once
#include "ECS/EntityManager.h"

namespace WanderSystem {

  // Update wander direction and set velocity for all wandering entities
  // Works on any entity with TRANSFORM | WANDER | SPEED
  // Skips entities that are currently dragging (their movement is handled by DragSystem)
  void Update(EntityManager& em, float deltaTime);

}