#pragma once
#include "ECS/EntityManager.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "ECS/systems/grids/PheromoneGrid.h"
#include "ECS/systems/grids/ColonyPheromoneManager.h"

namespace AntSystem {

  // Update all ant AI - handles state machine, pheromone deposits, stuck detection
  // State-specific logic is delegated to AntStates/* handlers
  void Update(EntityManager& em, SpatialGrid& grid, PheromoneGrid& pheromones,
    ColonyPheromoneManager& colonyPheromones, float deltaTime);

}