#include "ECS/systems/SpiderSystem.h"
#include "ECS/Components.h"
#include "ECS/Constants.h"

namespace SpiderSystem {

  // Distance at which spider stops to attack (should match collision radius)
  constexpr float ATTACK_RANGE = 15.0f;

  void Update(EntityManager& em, SpatialGrid& grid, float deltaTime) {
    auto spiders = em.GetEntitiesWithComponents(SPIDER | TRANSFORM | WANDER | SPEED | DETECTION);

    for (Entity spider : spiders) {
      auto& transform = em.GetComponent<CTransform>(spider);
      auto& wander = em.GetComponent<CWander>(spider);
      auto& speed = em.GetComponent<CSpeed>(spider);
      auto& detection = em.GetComponent<CDetection>(spider);
      auto& combat = em.GetComponent<CCombat>(spider);

      if (combat.attackTimer > 0.0f) {
        combat.attackTimer -= deltaTime;
      }

      // Find nearest ant to hunt
      Entity nearestAnt = grid.QueryNearest(transform.position, detection.radius,
        ANT | TRANSFORM, em);

      if (nearestAnt != INVALID_ENTITY) {
        Vec2 antPos = em.GetComponent<CTransform>(nearestAnt).position;
        Vec2 toAnt = antPos - transform.position;
        float dist = toAnt.Length();

        if (dist <= ATTACK_RANGE) {
          // Close enough to attack - stop moving (collision system handles damage)
          transform.velocity = Vec2(0.0f, 0.0f);
        }
        else if (dist > 0.001f) {
          // Hunt the ant!
          wander.direction = toAnt / dist;
          transform.velocity = wander.direction * speed.value;
        }
      }
      else {
        // No ant nearby - wander (WanderSystem already set direction)
        transform.velocity = wander.direction * speed.value;
      }
    }
  }

}