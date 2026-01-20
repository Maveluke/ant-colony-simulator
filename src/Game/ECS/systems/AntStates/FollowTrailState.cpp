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

    // State Update - uses CACHED nearby query results
    void Update(AntContext& ctx) {
      // Check for spider threat (from CACHED query)
      if (ctx.nearby.hasSpider()) {
        ctx.ant.state = AntState::FLEE;
        return;
      }

      // Check for nearby enemy ants - attack! (from CACHED query)
      if (ctx.nearby.hasEnemyAnt()) {
        ctx.ant.state = AntState::ATTACK;
        return;
      }

      // Check for actual food (from CACHED query)
      if (ctx.nearby.hasFood()) {
        ctx.target.entity = ctx.nearby.nearestFood;
        Vec2 foodPos = ctx.em.GetComponent<CTransform>(ctx.nearby.nearestFood).position;
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
