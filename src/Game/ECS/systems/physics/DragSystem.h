#pragma once
#include "ECS/EntityManager.h"
#include "ECS/Components.h"
#include "ECS/systems/grids/PheromoneGrid.h"

namespace DragSystem {

  // Get drag efficiency (0.0 to 1.0) based on dragger count vs weight
  float GetEfficiency(const CDraggable& draggable);

  // Check if an entity is currently dragging something
  bool IsDragging(EntityManager& em, Entity entity);

  // Update drag groups - samples pheromone from food position for unified direction
  // All draggers in a group move together based on single Monte Carlo sample
  void Update(EntityManager& em, PheromoneGrid& pheromones, float deltaTime);

  // Try to start dragging - returns true if successful
  bool StartDragging(EntityManager& em, Entity dragger, Entity target);

  // Stop dragging
  void StopDragging(EntityManager& em, Entity dragger);

}