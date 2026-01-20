#pragma once
#include "ECS/EntityManager.h"
#include "ECS/Components.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "ECS/systems/grids/PheromoneGrid.h"
#include "ECS/systems/grids/ColonyPheromoneManager.h"
#include "ECS/systems/helpers/NearbyQuery.h"

// Context struct passed to all ant state handlers
// Bundles entity, system references, and component references
// Created once per entity per frame to avoid repeated lookups
struct AntContext {
  // Entity ID
  Entity entity;

  // System references
  EntityManager& em;
  SpatialGrid& grid;
  PheromoneGrid& pheromones;          // Shared pheromones (FOOD, ALARM, PLAYER)
  ColonyPheromoneManager& colonyPheromones;  // Per-team HOME pheromones
  float deltaTime;

  // Component references (fetched once per entity)
  CAnt& ant;
  CTransform& transform;
  CWander& wander;
  CSpeed& speed;
  CDetection& detection;
  CTarget& target;
  CCombat& combat;

  // Cached nearby query result (computed once per ant per frame)
  NearbyEntities nearby;

  // Constructor
  AntContext(
    Entity e,
    EntityManager& em_,
    SpatialGrid& grid_,
    PheromoneGrid& pheromones_,
    ColonyPheromoneManager& colonyPheromones_,
    float dt,
    CAnt& ant_,
    CTransform& transform_,
    CWander& wander_,
    CSpeed& speed_,
    CDetection& detection_,
    CTarget& target_,
    CCombat& combat_,
    const NearbyEntities& nearby_
  )
    : entity(e)
    , em(em_)
    , grid(grid_)
    , pheromones(pheromones_)
    , colonyPheromones(colonyPheromones_)
    , deltaTime(dt)
    , ant(ant_)
    , transform(transform_)
    , wander(wander_)
    , speed(speed_)
    , detection(detection_)
    , target(target_)
    , combat(combat_)
    , nearby(nearby_)
  {
  }
};
