#pragma once
#include "../EntityManager.h"

namespace DragSystem {

  // Update dragged entities - calculates velocity based on draggers
  void Update(EntityManager& em, float deltaTime);

  // Try to start dragging - returns true if successful
  bool StartDragging(EntityManager& em, Entity dragger, Entity target);

  // Stop dragging
  void StopDragging(EntityManager& em, Entity dragger);

}