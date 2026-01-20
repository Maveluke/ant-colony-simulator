#include "ECS/systems/AntStates/FollowTrailState.h"
#include "ECS/systems/helpers/PheromoneNavigator.h"
#include "ECS/Constants.h"

namespace AntStates {
  namespace FollowTrail {

    // Constants
    constexpr float FOOD_TRAIL_THRESHOLD = 0.0f;
    constexpr float DIRECTION_LERP_FACTOR = 0.5f;  // Direction smoothing

    // Local Helpers
    static bool TickEscapeTimer(CWander& wander, float deltaTime) {
      if (wander.escapeTimer > 0.0f) {
        wander.escapeTimer -= deltaTime;
        return true;
      }
      return false;
    }

    static bool ShouldResamplePheromone(CWander& wander, float deltaTime) {
      wander.pheromoneTimer -= deltaTime;
      if (wander.pheromoneTimer <= 0.0f) {
        wander.pheromoneTimer = PheromoneNavigator::SAMPLE_INTERVAL;
        return true;
      }
      return false;
    }

    static void MoveToward(CTransform& transform, CWander& wander,
      float speed, const Vec2& targetPos) {
      Vec2 toTarget = targetPos - transform.position;
      float dist = toTarget.Length();

      if (dist > 0.001f) {
        wander.direction = toTarget / dist;
        transform.velocity = wander.direction * speed;
      }
    }

    static void ApplyDirectionAsVelocity(CTransform& transform, const CWander& wander,
      float speed) {
      transform.velocity = wander.direction * speed;
    }

    static Entity FindNearbySpider(EntityManager& em, SpatialGrid& grid,
      const Vec2& position, float radius) {
      return grid.QueryNearest(position, radius, SPIDER | TRANSFORM, em);
    }

    static bool CanDragFood(EntityManager& em, Entity food) {
      if (!em.HasComponents(DRAGGABLE, food)) return false;
      auto& draggable = em.GetComponent<CDraggable>(food);
      return draggable.draggerCount < draggable.maxDraggers;
    }

    static Entity FindAvailableFood(EntityManager& em, SpatialGrid& grid,
      const Vec2& position, float radius) {
      auto nearby = grid.Query(position, radius);

      float nearestDistSq = radius * radius + 1.0f;
      Entity nearestFood = INVALID_ENTITY;

      for (Entity e : nearby) {
        if (!em.HasComponents(FOOD | TRANSFORM | CIRCLE_COLLIDER, e)) continue;
        if (!CanDragFood(em, e)) continue;

        const Vec2& foodPos = em.GetComponent<CTransform>(e).position;
        float distSq = position.DistanceSquared(foodPos);

        if (distSq < nearestDistSq) {
          nearestDistSq = distSq;
          nearestFood = e;
        }
      }

      return nearestFood;
    }

    // Find nearest enemy ant (different team) within radius
    static Entity FindNearbyEnemyAnt(EntityManager& em, SpatialGrid& grid,
      const Vec2& position, float radius, TeamId myTeam) {
      auto nearby = grid.Query(position, radius);

      float closestDistSq = radius * radius;
      Entity closestEnemy = INVALID_ENTITY;

      for (Entity other : nearby) {
        if (!em.HasComponents(ANT | TRANSFORM, other)) continue;

        auto& otherAnt = em.GetComponent<CAnt>(other);

        // Skip same team or no team
        if (otherAnt.teamId == myTeam || otherAnt.teamId == TEAM_NONE) continue;

        auto& otherTransform = em.GetComponent<CTransform>(other);
        float distSq = position.DistanceSquared(otherTransform.position);

        if (distSq < closestDistSq) {
          closestDistSq = distSq;
          closestEnemy = other;
        }
      }

      return closestEnemy;
    }

    // Aggro radius for enemy ant detection (smaller than full detection for performance)
    constexpr float ENEMY_AGGRO_RADIUS = 30.0f;

    // State Update
    void Update(AntContext& ctx) {
      // Check for spider threat (direct sighting only - we're focused on food)
      Entity spider = FindNearbySpider(ctx.em, ctx.grid,
        ctx.transform.position, ctx.detection.radius);
      if (spider != INVALID_ENTITY) {
        ctx.ant.state = AntState::FLEE;
        return;
      }

      // Check for nearby enemy ants - attack! (use smaller aggro radius for performance)
      float aggroRadius = (ctx.detection.radius < ENEMY_AGGRO_RADIUS) ? ctx.detection.radius : ENEMY_AGGRO_RADIUS;
      Entity enemyAnt = FindNearbyEnemyAnt(ctx.em, ctx.grid,
        ctx.transform.position, aggroRadius, ctx.ant.teamId);
      if (enemyAnt != INVALID_ENTITY) {
        ctx.ant.state = AntState::ATTACK;
        return;
      }

      // Check for actual food
      Entity nearestFood = FindAvailableFood(ctx.em, ctx.grid,
        ctx.transform.position, ctx.detection.radius);
      if (nearestFood != INVALID_ENTITY) {
        ctx.target.entity = nearestFood;
        Vec2 foodPos = ctx.em.GetComponent<CTransform>(nearestFood).position;
        MoveToward(ctx.transform, ctx.wander, ctx.speed.value, foodPos);
        return;
      }

      // Skip pheromone sampling while escaping
      if (TickEscapeTimer(ctx.wander, ctx.deltaTime)) {
        ApplyDirectionAsVelocity(ctx.transform, ctx.wander, ctx.speed.value);
        return;
      }

      // Check if trail is still strong enough
      float currentFoodPheromone = ctx.pheromones.GetIntensity(PHEROMONE_FOOD,
        ctx.transform.position);
      if (currentFoodPheromone < FOOD_TRAIL_THRESHOLD) {
        ctx.ant.state = AntState::WANDER;
        ctx.target.entity = INVALID_ENTITY;
        return;
      }

      // Resample pheromone direction periodically
      if (ShouldResamplePheromone(ctx.wander, ctx.deltaTime)) {
        Vec2 trailDir = PheromoneNavigator::SampleBestDirection(
          ctx.pheromones, PHEROMONE_FOOD, ctx.transform.position, ctx.wander.direction,
          PheromoneNavigator::WIDE_CONE_ANGLE);
        if (trailDir.LengthSquared() > 0.0001f) {
          // Smooth direction change to prevent jittery movement
          ctx.wander.direction = ctx.wander.direction.Lerp(trailDir, DIRECTION_LERP_FACTOR);
          ctx.wander.direction.Normalize();
        }
      }

      ApplyDirectionAsVelocity(ctx.transform, ctx.wander, ctx.speed.value);
    }
  }
}
