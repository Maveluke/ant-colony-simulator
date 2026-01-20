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

    // Check threats using CACHED nearby query results (no more spatial queries here!)
    static bool CheckAndHandleThreat(AntContext& ctx) {
      // Direct spider sighting takes priority - flee!
      if (ctx.nearby.hasSpider()) {
        ctx.ant.state = AntState::FLEE;
        return true;
      }

      // Check for nearby enemy ants - attack!
      if (ctx.nearby.hasEnemyAnt()) {
        ctx.ant.state = AntState::ATTACK;
        return true;
      }

      // Check alarm pheromone
      AlarmResponse response = EvaluateAlarmResponse(ctx.pheromones, ctx.transform.position);
      if (response == AlarmResponse::ATTACK) {
        ctx.ant.state = AntState::ATTACK;
        return true;
      }
      else if (response == AlarmResponse::FLEE) {
        ctx.ant.state = AntState::FLEE;
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

      // Check for threats using CACHED results
      if (CheckAndHandleThreat(ctx)) {
        return;
      }

      // Look for nearby food (from CACHED query)
      if (ctx.nearby.hasFood()) {
        ctx.target.entity = ctx.nearby.nearestFood;
        Vec2 foodPos = ctx.em.GetComponent<CTransform>(ctx.nearby.nearestFood).position;
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
