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

    static bool CheckAndHandleThreat(CAnt& ant, EntityManager& em, SpatialGrid& grid,
      PheromoneGrid& pheromones,
      const Vec2& position, float detectionRadius) {
      // Direct spider sighting takes priority
      Entity spider = FindNearbySpider(em, grid, position, detectionRadius);
      if (spider != INVALID_ENTITY) {
        ant.state = AntState::FLEE;
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

      // No food or trail - let WanderSystem handle movement
      ctx.target.entity = INVALID_ENTITY;
    }

  }
}
