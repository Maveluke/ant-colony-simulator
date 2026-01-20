#include "ECS/systems/AntCollisionSystem.h"
#include "ECS/systems/physics/DragSystem.h"
#include "ECS/Components.h"
#include "ECS/Constants.h"

namespace AntCollisionSystem {

  void HandleCollisions(EntityManager& em, EventBuffer& events) {
    // Ant <-> Food collisions (start dragging)
    for (const auto& collision : events.Get<AntFoodCollision>()) {
      Entity ant = collision.ant;
      Entity food = collision.food;

      if (!em.HasComponents(ANT | TRANSFORM | DRAGGING, ant)) continue;
      if (!em.HasComponents(FOOD | DRAGGABLE, food)) continue;

      auto& antComp = em.GetComponent<CAnt>(ant);
      auto& dragging = em.GetComponent<CDragging>(ant);

      // Only pick up if wandering/following trail and not already dragging
      if (antComp.state != AntState::WANDER && antComp.state != AntState::FOLLOW_TRAIL) continue;
      if (dragging.target != INVALID_ENTITY) continue;

      if (DragSystem::StartDragging(em, ant, food)) {
        if (em.HasComponents(TARGET, ant)) {
          em.GetComponent<CTarget>(ant).entity = INVALID_ENTITY;
        }
        antComp.state = AntState::FORAGE;
      }
    }

    // Food <-> Colony collisions (deposit food)
    for (const auto& collision : events.Get<FoodColonyCollision>()) {
      Entity food = collision.food;
      Entity colony = collision.colony;

      if (!em.HasComponents(FOOD | DRAGGABLE, food)) continue;
      if (!em.HasComponents(COLONY, colony)) continue;

      auto& foodComp = em.GetComponent<CFood>(food);
      auto& draggable = em.GetComponent<CDraggable>(food);
      auto& colonyComp = em.GetComponent<CColony>(colony);

      // Transfer food to colony
      colonyComp.storedFood += foodComp.amount;

      // Stop all draggers and set them back to wander
      for (int i = 0; i < draggable.draggerCount; i++) {
        Entity dragger = draggable.draggers[i];
        if (!em.HasComponents(ANT | DRAGGING, dragger)) continue;

        em.GetComponent<CDragging>(dragger).target = INVALID_ENTITY;
        em.GetComponent<CAnt>(dragger).state = AntState::WANDER;
      }

      em.DeleteEntity(food);
    }

    // Ant <-> Spider collisions (combat)
    for (const auto& collision : events.Get<AntSpiderCollision>()) {
      Entity ant = collision.ant;
      Entity spider = collision.spider;

      if (!em.HasComponents(ANT | HEALTH | COMBAT, ant)) continue;
      if (!em.HasComponents(SPIDER | HEALTH | COMBAT, spider)) continue;

      auto& antComp = em.GetComponent<CAnt>(ant);
      auto& antHealth = em.GetComponent<CHealth>(ant);
      auto& antCombat = em.GetComponent<CCombat>(ant);
      auto& spiderHealth = em.GetComponent<CHealth>(spider);
      auto& spiderCombat = em.GetComponent<CCombat>(spider);

      // Spider attacks ant
      if (spiderCombat.attackTimer <= 0.0f) {
        antHealth.current -= spiderCombat.attackDamage;
        spiderCombat.attackTimer = spiderCombat.attackCooldown;

        if (antHealth.current <= 0.0f) {
          em.DeleteEntity(ant);
          continue;
        }
      }

      // Ant fights back if in ATTACK state
      if (antComp.state == AntState::ATTACK && antCombat.attackTimer <= 0.0f) {
        spiderHealth.current -= antCombat.attackDamage;
        antCombat.attackTimer = antCombat.attackCooldown;

        if (spiderHealth.current <= 0.0f) {
          // Convert dead spider to food
          em.AddComponent(DRAGGABLE, spider);
          auto& draggable = em.GetComponent<CDraggable>(spider);
          draggable.weight = 10.0f;
          draggable.maxDraggers = MAX_DRAGGERS;
          draggable.draggerCount = 0;

          em.RemoveComponent(SPIDER, spider);
          em.AddComponent(FOOD, spider);
          em.GetComponent<CFood>(spider).amount = 150.0f;
        }
      }

      // Non-combat ants enter FLEE state
      if (antComp.state != AntState::FLEE && antComp.state != AntState::ATTACK) {
        antComp.state = AntState::FLEE;
      }
    }
  }
}