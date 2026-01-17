#pragma once
#include "ECS/EntityManager.h"
#include "SpatialGrid.h"

namespace AntSystem {

  // Call once per frame to update all ant behavior
  void Update(EntityManager& em, SpatialGrid& grid, float deltaTime,
    float worldWidth, float worldHeight);

}