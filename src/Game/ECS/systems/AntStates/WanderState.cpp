#include "ECS/systems/AntStates/WanderState.h"
#include "ECS/systems/helpers/PheromoneNavigator.h"
#include "ECS/Constants.h"

namespace AntStates {
  namespace Wander {

    // Local Helpers
    static bool TickEscapeTimer(CWander& wander, float deltaTime) {
      if (wander.escapeTimer > 0.0f) {
        wander.escapeTimer -= deltaTime;
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

    // Alarm thresholds
    constexpr float ALARM_ATTACK_THRESHOLD = 20.0f;

    // Direction smoothing - blend factor (0 = no change, 1 = instant snap)
    constexpr float DIRECTION_LERP_FACTOR = 0.5f;

    enum class AlarmResponse { NONE, FLEE, ATTACK };

    static AlarmResponse EvaluateAlarmResponse(const PheromoneGrid& pheromones,
      const Vec2& position) {
      float alarmIntensity = pheromones.GetIntensity(PHEROMONE_ALARM, position);

      if (alarmIntensity > PheromoneNavigator::DEFAULT_MIN_INTENSITY) {
        if (alarmIntensity >= ALARM_ATTACK_THRESHOLD) {
          return AlarmResponse::ATTACK;
        }
        return AlarmResponse::FLEE;
      }
      return AlarmResponse::NONE;
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

    static bool CheckAndHandleThreat(CAnt& ant, EntityManager& em, SpatialGrid& grid,
      PheromoneGrid& pheromones,
      const Vec2& position, float detectionRadius) {
      // Direct spider sighting takes priority - flee!
      Entity spider = FindNearbySpider(em, grid, position, detectionRadius);
      if (spider != INVALID_ENTITY) {
        ant.state = AntState::FLEE;
        return true;
      }

      // Check for nearby enemy ants - attack! (use smaller aggro radius for performance)
      float aggroRadius = (detectionRadius < ENEMY_AGGRO_RADIUS) ? detectionRadius : ENEMY_AGGRO_RADIUS;
      Entity enemyAnt = FindNearbyEnemyAnt(em, grid, position, aggroRadius, ant.teamId);
      if (enemyAnt != INVALID_ENTITY) {
        ant.state = AntState::ATTACK;
        return true;
      }

      // Check alarm pheromone
      AlarmResponse response = EvaluateAlarmResponse(pheromones, position);
      if (response == AlarmResponse::ATTACK) {
        ant.state = AntState::ATTACK;
        return true;
      }
      else if (response == AlarmResponse::FLEE) {
        ant.state = AntState::FLEE;
        return true;
      }

      return false;
    }

    // State Update
    void Update(AntContext& ctx) {
      // Skip pheromone logic while escaping
      if (TickEscapeTimer(ctx.wander, ctx.deltaTime)) {
        ctx.target.entity = INVALID_ENTITY;
        return;
      }

      // Check for threats (spider or alarm pheromone)
      if (CheckAndHandleThreat(ctx.ant, ctx.em, ctx.grid, ctx.pheromones,
        ctx.transform.position, ctx.detection.radius)) {
        return;
      }

      // Look for nearby food
      Entity nearestFood = FindAvailableFood(ctx.em, ctx.grid,
        ctx.transform.position, ctx.detection.radius);
      if (nearestFood != INVALID_ENTITY) {
        ctx.target.entity = nearestFood;
        Vec2 foodPos = ctx.em.GetComponent<CTransform>(nearestFood).position;
        MoveToward(ctx.transform, ctx.wander, ctx.speed.value, foodPos);
        return;
      }

      // Check for FOOD pheromone trail
      Vec2 foodPheromone = PheromoneNavigator::SampleBestDirection(
        ctx.pheromones, PHEROMONE_FOOD, ctx.transform.position, ctx.wander.direction,
        PheromoneNavigator::WIDE_CONE_ANGLE);
      if (foodPheromone.LengthSquared() > 0.0001f) {
        ctx.ant.state = AntState::FOLLOW_TRAIL;
        return;
      }

      // Check for PLAYER pheromone trail - follow where the player draws!
      Vec2 playerPheromone = PheromoneNavigator::SampleBestDirection(
        ctx.pheromones, PHEROMONE_PLAYER, ctx.transform.position, ctx.wander.direction,
        PheromoneNavigator::WIDE_CONE_ANGLE);
      if (playerPheromone.LengthSquared() > 0.0001f) {
        // Smooth direction change to prevent jittery movement
        ctx.wander.direction = ctx.wander.direction.Lerp(playerPheromone, DIRECTION_LERP_FACTOR);
        ctx.wander.direction.Normalize();
        ctx.transform.velocity = ctx.wander.direction * ctx.speed.value;
        return;
      }

      // No food or trail - let WanderSystem handle movement
      ctx.target.entity = INVALID_ENTITY;
    }

  }
}
