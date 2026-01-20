#pragma once
#include "ECS/EntityManager.h"
#include "ECS/Components.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "ECS/systems/grids/PheromoneGrid.h"

// Context struct passed to all ant state handlers
// Bundles entity, system references, and component references
// Created once per entity per frame to avoid repeated lookups
struct AntContext {
  // Entity ID
  Entity entity;

  // System references
  EntityManager& em;
  SpatialGrid& grid;
  PheromoneGrid& pheromones;
  float deltaTime;

  // Component references (fetched once per entity)
  CAnt& ant;
  CTransform& transform;
  CWander& wander;
  CSpeed& speed;
  CDetection& detection;
  CTarget& target;
  CCombat& combat;

  // Constructor
  AntContext(
    Entity e,
    EntityManager& em_,
    SpatialGrid& grid_,
    PheromoneGrid& pheromones_,
    float dt,
    CAnt& ant_,
    CTransform& transform_,
    CWander& wander_,
    CSpeed& speed_,
    CDetection& detection_,
    CTarget& target_,
    CCombat& combat_
  )
    : entity(e)
    , em(em_)
    , grid(grid_)
    , pheromones(pheromones_)
    , deltaTime(dt)
    , ant(ant_)
    , transform(transform_)
    , wander(wander_)
    , speed(speed_)
    , detection(detection_)
    , target(target_)
    , combat(combat_)
  {
  }
};
