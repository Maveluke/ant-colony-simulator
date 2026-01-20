#include "ECS/systems/AntStates/AttackState.h"
#include "ECS/systems/helpers/PheromoneNavigator.h"
#include "ECS/Constants.h"

namespace AntStates {
  namespace Attack {

    // Constants
    constexpr float ALARM_DEPOSIT_AMOUNT = 10.0f;
    constexpr float ALARM_DEPOSIT_RADIUS = 8.0f;
    constexpr float ALARM_ATTACK_THRESHOLD = 20.0f;
    constexpr float ALARM_RETREAT_THRESHOLD = ALARM_ATTACK_THRESHOLD * 0.5f;

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

    static void ApplyDirectionAsVelocity(CTransform& transform, const CWander& wander,
      float speed) {
      transform.velocity = wander.direction * speed;
    }

    // State Update - uses CACHED nearby query results
    void Update(AntContext& ctx) {
      // Check if swarm has dispersed - should we retreat?
      float alarmIntensity = ctx.pheromones.GetIntensity(PHEROMONE_ALARM, ctx.transform.position);
      if (alarmIntensity < ALARM_RETREAT_THRESHOLD) {
        ctx.ant.state = AntState::FLEE;
        return;
      }

      // Look for visible spider first (higher priority threat) - from CACHED query
      if (ctx.nearby.hasSpider()) {
        // Keep depositing ALARM to maintain swarm
        ctx.pheromones.DepositRadius(PHEROMONE_ALARM, ctx.transform.position,
          ALARM_DEPOSIT_RADIUS, ALARM_DEPOSIT_AMOUNT);
        Vec2 spiderPos = ctx.em.GetComponent<CTransform>(ctx.nearby.nearestSpider).position;
        MoveToward(ctx.transform, ctx.wander, ctx.speed.value, spiderPos);
        return;
      }

      // Look for nearby enemy ants - from CACHED query
      if (ctx.nearby.hasEnemyAnt()) {
        // Deposit alarm to attract allies to the fight
        ctx.pheromones.DepositRadius(PHEROMONE_ALARM, ctx.transform.position,
          ALARM_DEPOSIT_RADIUS, ALARM_DEPOSIT_AMOUNT * 0.5f);  // Less than spider
        Vec2 enemyPos = ctx.em.GetComponent<CTransform>(ctx.nearby.nearestEnemyAnt).position;
        MoveToward(ctx.transform, ctx.wander, ctx.speed.value, enemyPos);
        return;
      }

      // No visible threats - follow alarm pheromone to find the fight
      if (TickEscapeTimer(ctx.wander, ctx.deltaTime)) {
        ApplyDirectionAsVelocity(ctx.transform, ctx.wander, ctx.speed.value);
        return;
      }

      Vec2 alarmDir = PheromoneNavigator::SampleBestDirection(
        ctx.pheromones, PHEROMONE_ALARM, ctx.transform.position, ctx.wander.direction,
        PheromoneNavigator::WIDE_CONE_ANGLE);

      if (alarmDir.LengthSquared() > 0.0001f) {
        ctx.wander.direction = alarmDir;  // Move TOWARD alarm (where the fight is)
        ApplyDirectionAsVelocity(ctx.transform, ctx.wander, ctx.speed.value);
      }
      else {
        // No alarm detected anymore, back to wandering
        ctx.ant.state = AntState::WANDER;
      }
    }

  }
}
