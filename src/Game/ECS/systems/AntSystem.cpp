#include "ECS/systems/AntSystem.h"
#include "ECS/systems/AntContext.h"
#include "ECS/systems/AntStates/AntStateHandlers.h"
#include "ECS/systems/helpers/StuckDetector.h"
#include "ECS/systems/physics/DragSystem.h"
#include "ECS/Components.h"
#include "ECS/Constants.h"

namespace AntSystem {

  // =============================================================================
  // Constants
  // =============================================================================

  constexpr float HOME_PHEROMONE_DEPOSIT = 10.0f;

  // =============================================================================
  // Context Builder
  // =============================================================================

  static AntContext BuildContext(
    Entity e,
    EntityManager& em,
    SpatialGrid& grid,
    PheromoneGrid& pheromones,
    ColonyPheromoneManager& colonyPheromones,
    float deltaTime
  ) {
    return AntContext(
      e,
      em,
      grid,
      pheromones,
      colonyPheromones,
      deltaTime,
      em.GetComponent<CAnt>(e),
      em.GetComponent<CTransform>(e),
      em.GetComponent<CWander>(e),
      em.GetComponent<CSpeed>(e),
      em.GetComponent<CDetection>(e),
      em.GetComponent<CTarget>(e),
      em.GetComponent<CCombat>(e)
    );
  }

  // =============================================================================
  // Main Update Loop
  // =============================================================================

  void Update(EntityManager& em, SpatialGrid& grid, PheromoneGrid& pheromones,
    ColonyPheromoneManager& colonyPheromones, float deltaTime) {

    auto ants = em.GetEntitiesWithComponents(ANT | TRANSFORM | WANDER | SPEED);

    for (Entity e : ants) {
      // Build context once per entity
      AntContext ctx = BuildContext(e, em, grid, pheromones, colonyPheromones, deltaTime);

      // Count down attack timer
      if (ctx.combat.attackTimer > 0.0f) {
        ctx.combat.attackTimer -= deltaTime;
      }

      // Check if ant is stuck (skip for dragging ants - handled in DragSystem)
      bool stuck = false;
      if (!DragSystem::IsDragging(em, e)) {
        stuck = StuckDetector::CheckAndEscapeAnt(ctx.wander, ctx.transform.position, deltaTime);
      }

      // Deposit HOME pheromone to own team's grid while exploring (not dragging food)
      if (!stuck && (ctx.ant.state == AntState::WANDER || ctx.ant.state == AntState::FOLLOW_TRAIL)) {
        colonyPheromones.DepositHome(ctx.ant.teamId, ctx.transform.position, HOME_PHEROMONE_DEPOSIT);
      }

      // Dispatch to state handler
      AntStateHandlers::Update(ctx);
    }
  }

}