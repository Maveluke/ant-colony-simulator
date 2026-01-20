#pragma once
#include "ECS/EntityManager.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "ECS/Components.h"
#include <cfloat>

/**
 * Result of a combined spatial query for ant AI.
 * Single query pass classifies all nearby entities by type.
 */
struct NearbyEntities {
  Entity nearestSpider = INVALID_ENTITY;
  float spiderDistSq = FLT_MAX;

  Entity nearestEnemyAnt = INVALID_ENTITY;
  float enemyAntDistSq = FLT_MAX;

  Entity nearestFood = INVALID_ENTITY;
  float foodDistSq = FLT_MAX;

  bool hasSpider() const { return nearestSpider != INVALID_ENTITY; }
  bool hasEnemyAnt() const { return nearestEnemyAnt != INVALID_ENTITY; }
  bool hasFood() const { return nearestFood != INVALID_ENTITY; }
};

namespace NearbyQuery {

  /**
   * Single-pass query that finds nearest spider, enemy ant, and available food.
   * Much faster than doing 3 separate queries!
   * Uses zero-allocation callback pattern for spatial query.
   *
   * @param em           Entity manager
   * @param grid         Spatial grid
   * @param position     Query center position
   * @param radius       Query radius
   * @param myTeam       The querying ant's team (to identify enemies)
   * @param aggroRadius  Smaller radius for enemy ant detection (performance)
   */
  inline NearbyEntities QueryAll(
    EntityManager& em,
    const SpatialGrid& grid,
    const Vec2& position,
    float radius,
    TeamId myTeam,
    float aggroRadius
  ) {
    NearbyEntities result;

    float aggroRadiusSq = aggroRadius * aggroRadius;

    // Zero-allocation spatial query using callback
    grid.QueryEach(position, radius, [&](Entity e) {
      // Get transform (all entities in grid have TRANSFORM)
      const Vec2& entityPos = em.GetComponent<CTransform>(e).position;
      float distSq = position.DistanceSquared(entityPos);

      // Check if it's a spider
      if (em.HasComponents(SPIDER, e)) {
        if (distSq < result.spiderDistSq) {
          result.nearestSpider = e;
          result.spiderDistSq = distSq;
        }
        return;  // Spider can't be food or ant
      }

      // Check if it's an enemy ant (within aggro radius)
      if (distSq <= aggroRadiusSq && em.HasComponents(ANT, e)) {
        auto& otherAnt = em.GetComponent<CAnt>(e);
        // Different team and valid team
        if (otherAnt.teamId != myTeam && otherAnt.teamId != TEAM_NONE) {
          if (distSq < result.enemyAntDistSq) {
            result.nearestEnemyAnt = e;
            result.enemyAntDistSq = distSq;
          }
        }
        return;  // Ant can't be food
      }

      // Check if it's available food
      if (em.HasComponents(FOOD | DRAGGABLE, e)) {
        auto& draggable = em.GetComponent<CDraggable>(e);
        // Can we pick it up?
        if (draggable.draggerCount < draggable.maxDraggers) {
          if (distSq < result.foodDistSq) {
            result.nearestFood = e;
            result.foodDistSq = distSq;
          }
        }
      }
      });

    return result;
  }

}
