#include "ECS/systems/AntSystem.h"
#include "ECS/systems/AntContext.h"
#include "ECS/systems/AntStates/AntStateHandlers.h"
#include "ECS/systems/helpers/StuckDetector.h"
#include "ECS/systems/helpers/NearbyQuery.h"
#include "ECS/systems/physics/DragSystem.h"
#include "ECS/Components.h"
#include "ECS/Constants.h"

namespace AntSystem {

  // =============================================================================
  // Constants
  // =============================================================================

  constexpr float HOME_PHEROMONE_DEPOSIT = 10.0f;
  constexpr float ENEMY_AGGRO_RADIUS = 30.0f;  // Smaller radius for enemy detection

  // =============================================================================
  // Context Builder (now includes nearby query)
  // =============================================================================

  static AntContext BuildContext(
    Entity e,
    EntityManager& em,
    SpatialGrid& grid,
    PheromoneGrid& pheromones,
    ColonyPheromoneManager& colonyPheromones,
    float deltaTime
  ) {
    auto& ant = em.GetComponent<CAnt>(e);
    auto& transform = em.GetComponent<CTransform>(e);
    auto& detection = em.GetComponent<CDetection>(e);

    // Single spatial query for all nearby entities - MAJOR OPTIMIZATION
    NearbyEntities nearby = NearbyQuery::QueryAll(
      em, grid, transform.position, detection.radius, ant.teamId, ENEMY_AGGRO_RADIUS
    );

    return AntContext(
      e,
      em,
      grid,
      pheromones,
      colonyPheromones,
      deltaTime,
      ant,
      transform,
      em.GetComponent<CWander>(e),
      em.GetComponent<CSpeed>(e),
      detection,
      em.GetComponent<CTarget>(e),
      em.GetComponent<CCombat>(e),
      nearby
    );
  }

  // =============================================================================
  // Main Update Loop
  // =============================================================================

  void Update(EntityManager& em, SpatialGrid& grid, PheromoneGrid& pheromones,
    ColonyPheromoneManager& colonyPheromones, float deltaTime) {

    const auto& ants = em.GetAnts();

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