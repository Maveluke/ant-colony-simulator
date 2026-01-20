#include "ECS/systems/AntStates/ForageState.h"
#include "ECS/Constants.h"

namespace AntStates {
  namespace Forage {

    // Constants
    constexpr float FOOD_PHEROMONE_DEPOSIT = 10.0f;

    // Local Helpers
    static bool TickEscapeTimer(CWander& wander, float deltaTime) {
      if (wander.escapeTimer > 0.0f) {
        wander.escapeTimer -= deltaTime;
        return true;
      }
      return false;
    }

    // State Update
    void Update(AntContext& ctx) {
      // Deposit FOOD pheromone as trail for other ants
      ctx.pheromones.Deposit(PHEROMONE_FOOD, ctx.transform.position, FOOD_PHEROMONE_DEPOSIT);

      // Skip direction override while escaping
      if (TickEscapeTimer(ctx.wander, ctx.deltaTime)) {
        return;
      }

      // If colony is visible, override direction toward it
      Entity colony = ctx.grid.QueryNearest(ctx.transform.position, ctx.detection.radius,
        COLONY | TRANSFORM, ctx.em);
      if (colony != INVALID_ENTITY) {
        Vec2 colonyPos = ctx.em.GetComponent<CTransform>(colony).position;
        Vec2 toColony = colonyPos - ctx.transform.position;
        float dist = toColony.Length();
        if (dist > 0.001f) {
          ctx.wander.direction = toColony / dist;
        }
      }
      // Else: DragSystem handles direction via Monte Carlo sampling
      // NOTE: We do NOT set velocity here - DragSystem applies unified velocity to drag group
    }

  }
}
