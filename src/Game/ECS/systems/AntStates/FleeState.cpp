#include "ECS/systems/AntStates/FleeState.h"
#include "ECS/systems/helpers/PheromoneNavigator.h"
#include "ECS/Constants.h"

namespace AntStates {
  namespace Flee {

    // Constants
    constexpr float FLEE_ALARM_DEPOSIT = 5.0f;
    constexpr float ALARM_ATTACK_THRESHOLD = 20.0f;
    constexpr float FLEE_SPEED_MULTIPLIER = 1.5f;

    // Local Helpers
    static bool TickEscapeTimer(CWander& wander, float deltaTime) {
      if (wander.escapeTimer > 0.0f) {
        wander.escapeTimer -= deltaTime;
        return true;
      }
      return false;
    }

    static void MoveAwayFrom(CTransform& transform, CWander& wander,
      float speed, const Vec2& threatPos,
      float speedMultiplier) {
      Vec2 awayFromThreat = transform.position - threatPos;
      float dist = awayFromThreat.Length();

      if (dist > 0.001f) {
        wander.direction = awayFromThreat / dist;
        transform.velocity = wander.direction * speed * speedMultiplier;
      }
    }

    static void ApplyDirectionAsVelocity(CTransform& transform, const CWander& wander,
      float speed, float speedMultiplier) {
      transform.velocity = wander.direction * speed * speedMultiplier;
    }

    // State Update
    void Update(AntContext& ctx) {
      // Deposit ALARM pheromone to alert nearby ants
      ctx.pheromones.Deposit(PHEROMONE_ALARM, ctx.transform.position, FLEE_ALARM_DEPOSIT);

      // Check if swarm has formed - should we fight back?
      float alarmIntensity = ctx.pheromones.GetIntensity(PHEROMONE_ALARM, ctx.transform.position);
      if (alarmIntensity >= ALARM_ATTACK_THRESHOLD) {
        ctx.ant.state = AntState::ATTACK;
        return;
      }

      // Look for visible spider (use cached nearby result)
      if (ctx.nearby.nearestSpider != INVALID_ENTITY) {
        Vec2 spiderPos = ctx.em.GetComponent<CTransform>(ctx.nearby.nearestSpider).position;
        MoveAwayFrom(ctx.transform, ctx.wander, ctx.speed.value, spiderPos, FLEE_SPEED_MULTIPLIER);
        return;
      }

      // No visible spider - use alarm pheromone to escape
      if (TickEscapeTimer(ctx.wander, ctx.deltaTime)) {
        ApplyDirectionAsVelocity(ctx.transform, ctx.wander, ctx.speed.value, FLEE_SPEED_MULTIPLIER);
        return;
      }

      // Sample alarm pheromone and move AWAY from it
      Vec2 alarmDir = PheromoneNavigator::SampleBestDirection(
        ctx.pheromones, PHEROMONE_ALARM, ctx.transform.position, ctx.wander.direction,
        PheromoneNavigator::WIDE_CONE_ANGLE);

      if (alarmDir.LengthSquared() > 0.0001f) {
        ctx.wander.direction = Vec2(-alarmDir.x, -alarmDir.y);  // Invert to flee
        ApplyDirectionAsVelocity(ctx.transform, ctx.wander, ctx.speed.value, FLEE_SPEED_MULTIPLIER);
      }
      else {
        // No alarm detected anymore, back to wandering
        ctx.ant.state = AntState::WANDER;
      }
    }

  }
}
