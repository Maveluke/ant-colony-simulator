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

      // Check if at least one dragger belongs to this colony's team
      bool validDeposit = false;
      for (int i = 0; i < draggable.draggerCount; i++) {
        Entity dragger = draggable.draggers[i];
        if (!em.HasComponents(ANT, dragger)) continue;

        auto& draggerAnt = em.GetComponent<CAnt>(dragger);
        if (draggerAnt.teamId == colonyComp.teamId) {
          validDeposit = true;
          break;
        }
      }

      // Only deposit if the ants belong to this colony
      if (!validDeposit) continue;

      // Transfer food to colony
      colonyComp.storedFood += foodComp.amount;

      // Stop all draggers and set them back to wander
      for (int i = 0; i < draggable.draggerCount; i++) {
        Entity dragger = draggable.draggers[i];
        if (!em.HasComponents(ANT | DRAGGING, dragger)) continue;

        em.GetComponent<CDragging>(dragger).target = INVALID_ENTITY;
        em.GetComponent<CAnt>(dragger).state = AntState::WANDER;

        // Flip direction 180° so ant faces back toward the food trail it just laid
        if (em.HasComponents(WANDER, dragger)) {
          auto& wander = em.GetComponent<CWander>(dragger);
          wander.direction = wander.direction * -1.0f;
        }
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
          // Drop anything the ant was dragging before deleting
          if (em.HasComponents(DRAGGING, ant)) {
            DragSystem::StopDragging(em, ant);
          }
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

    // Ant <-> Ant collisions (different colonies fighting)
    for (const auto& collision : events.Get<AntAntCollision>()) {
      Entity ant1 = collision.ant1;
      Entity ant2 = collision.ant2;

      if (!em.HasComponents(ANT | HEALTH | COMBAT | TRANSFORM, ant1)) continue;
      if (!em.HasComponents(ANT | HEALTH | COMBAT | TRANSFORM, ant2)) continue;

      auto& ant1Comp = em.GetComponent<CAnt>(ant1);
      auto& ant1Health = em.GetComponent<CHealth>(ant1);
      auto& ant1Combat = em.GetComponent<CCombat>(ant1);
      auto& ant2Comp = em.GetComponent<CAnt>(ant2);
      auto& ant2Health = em.GetComponent<CHealth>(ant2);
      auto& ant2Combat = em.GetComponent<CCombat>(ant2);

      // Both ants enter ATTACK state when encountering enemy
      if (ant1Comp.state != AntState::ATTACK) {
        ant1Comp.state = AntState::ATTACK;
        // Set target to enemy ant
        if (em.HasComponents(TARGET, ant1)) {
          em.GetComponent<CTarget>(ant1).entity = ant2;
        }
      }
      if (ant2Comp.state != AntState::ATTACK) {
        ant2Comp.state = AntState::ATTACK;
        if (em.HasComponents(TARGET, ant2)) {
          em.GetComponent<CTarget>(ant2).entity = ant1;
        }
      }

      // Ant1 attacks Ant2
      if (ant1Combat.attackTimer <= 0.0f) {
        ant2Health.current -= ant1Combat.attackDamage;
        ant1Combat.attackTimer = ant1Combat.attackCooldown;

        if (ant2Health.current <= 0.0f) {
          // Drop anything ant2 was dragging
          if (em.HasComponents(DRAGGING, ant2)) {
            DragSystem::StopDragging(em, ant2);
          }
          em.DeleteEntity(ant2);
          // Reset ant1 to wander after killing enemy
          ant1Comp.state = AntState::WANDER;
          if (em.HasComponents(TARGET, ant1)) {
            em.GetComponent<CTarget>(ant1).entity = INVALID_ENTITY;
          }
          continue;
        }
      }

      // Ant2 attacks Ant1
      if (ant2Combat.attackTimer <= 0.0f) {
        ant1Health.current -= ant2Combat.attackDamage;
        ant2Combat.attackTimer = ant2Combat.attackCooldown;

        if (ant1Health.current <= 0.0f) {
          // Drop anything ant1 was dragging
          if (em.HasComponents(DRAGGING, ant1)) {
            DragSystem::StopDragging(em, ant1);
          }
          em.DeleteEntity(ant1);
          // Reset ant2 to wander after killing enemy
          ant2Comp.state = AntState::WANDER;
          if (em.HasComponents(TARGET, ant2)) {
            em.GetComponent<CTarget>(ant2).entity = INVALID_ENTITY;
          }
          continue;
        }
      }
    }
  }
}