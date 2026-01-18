#include "AntSystem.h"
#include "../Components.h"
#include "../Constants.h"
#include <cstdlib>
#include <cmath>

namespace AntSystem {

  // Constants
  constexpr float ALARM_DEPOSIT_AMOUNT = 3.0f;
  constexpr float ALARM_DEPOSIT_RADIUS = 30.0f;
  constexpr float ALARM_ATTACK_THRESHOLD = 20.0f;

  // Handle WANDER state - look for food, override direction if found
  // WanderSystem handles default wandering behavior
  static void UpdateWander(Entity e, EntityManager& em, SpatialGrid& grid,
    PheromoneGrid& pheromones, float deltaTime) {
    auto& transform = em.GetComponent<CTransform>(e);
    auto& wander = em.GetComponent<CWander>(e);
    auto& speed = em.GetComponent<CSpeed>(e);
    auto& detection = em.GetComponent<CDetection>(e);
    auto& target = em.GetComponent<CTarget>(e);

    // Look for nearby food
    Entity nearestFood = grid.QueryNearest(transform.position, detection.radius,
      FOOD | TRANSFORM | CIRCLE_COLLIDER, em);

    if (nearestFood != INVALID_ENTITY) {
      // Found food! Target it and override direction toward it
      target.entity = nearestFood;
      Vec2 foodPos = em.GetComponent<CTransform>(nearestFood).position;
      Vec2 toFood = foodPos - transform.position;
      float dist = toFood.Length();

      if (dist > 0.001f) {
        wander.direction = toFood / dist;
        transform.velocity = wander.direction * speed.value;
      }
    }
    else {
      // No food nearby, let WanderSystem handle movement
      target.entity = INVALID_ENTITY;
    }
  }

  // Handle FORAGE state - returning home with food
  static void UpdateForage(Entity e, EntityManager& em, SpatialGrid& grid,
    PheromoneGrid& pheromones, float deltaTime) {
    auto& transform = em.GetComponent<CTransform>(e);
    auto& wander = em.GetComponent<CWander>(e);
    auto& speed = em.GetComponent<CSpeed>(e);
    auto& detection = em.GetComponent<CDetection>(e);

    // Deposit FOOD pheromone as the ants walk
    pheromones.Deposit(PHEROMONE_FOOD, transform.position, 5.0f);

    // Look for colony
    Entity colony = grid.QueryNearest(transform.position, detection.radius,
      COLONY | TRANSFORM, em);

    if (colony != INVALID_ENTITY) {
      // Found colony! Override direction toward it
      Vec2 colonyPos = em.GetComponent<CTransform>(colony).position;
      Vec2 toColony = colonyPos - transform.position;
      float dist = toColony.Length();

      if (dist > 0.001f) {
        wander.direction = toColony / dist;
        transform.velocity = wander.direction * speed.value;
      }
    }
    else {
      // No colony in detection range, follow HOME pheromone
      Vec2 homeDir = pheromones.SampleGradient(PHEROMONE_HOME, transform.position);

      if (homeDir.LengthSquared() > 0.0001f) {
        wander.direction = homeDir;
        transform.velocity = wander.direction * speed.value;
      }
      // Else: let WanderSystem handle it
    }
  }

  // Handle FLEE state - run away from spider
  static void UpdateFlee(Entity e, EntityManager& em, SpatialGrid& grid,
    PheromoneGrid& pheromones, float deltaTime) {
    auto& ant = em.GetComponent<CAnt>(e);
    auto& transform = em.GetComponent<CTransform>(e);
    auto& wander = em.GetComponent<CWander>(e);
    auto& speed = em.GetComponent<CSpeed>(e);
    auto& detection = em.GetComponent<CDetection>(e);

    // Look for nearby spider
    Entity spider = grid.QueryNearest(transform.position, detection.radius,
      SPIDER | TRANSFORM, em);

    if (spider != INVALID_ENTITY) {
      // Deposit ALARM pheromone to alert nearby ants
      pheromones.DepositRadius(PHEROMONE_ALARM, transform.position,
        ALARM_DEPOSIT_RADIUS, ALARM_DEPOSIT_AMOUNT);

      // Check alarm intensity - should we fight back?
      float alarmIntensity = pheromones.GetIntensity(PHEROMONE_ALARM, transform.position);
      if (alarmIntensity >= ALARM_ATTACK_THRESHOLD) {
        // Swarm detected! Switch to attack
        ant.state = AntState::ATTACK;
        return;
      }

      // Not enough backup - keep fleeing
      Vec2 spiderPos = em.GetComponent<CTransform>(spider).position;
      Vec2 awayFromSpider = transform.position - spiderPos;
      float dist = awayFromSpider.Length();

      if (dist > 0.001f) {
        wander.direction = awayFromSpider / dist;
      }

      // Flee faster!
      transform.velocity = wander.direction * speed.value * 1.5f;
    }
    else {
      // No spider nearby, go back to wandering
      ant.state = AntState::WANDER;
      // WanderSystem will handle velocity
    }
  }

  // Handle ATTACK state - move toward and attack spider
  static void UpdateAttack(Entity e, EntityManager& em, SpatialGrid& grid,
    PheromoneGrid& pheromones, float deltaTime) {
    auto& ant = em.GetComponent<CAnt>(e);
    auto& transform = em.GetComponent<CTransform>(e);
    auto& wander = em.GetComponent<CWander>(e);
    auto& speed = em.GetComponent<CSpeed>(e);
    auto& detection = em.GetComponent<CDetection>(e);

    // Look for nearby spider
    Entity spider = grid.QueryNearest(transform.position, detection.radius,
      SPIDER | TRANSFORM, em);

    if (spider != INVALID_ENTITY) {
      // Keep depositing ALARM to maintain swarm
      pheromones.DepositRadius(PHEROMONE_ALARM, transform.position,
        ALARM_DEPOSIT_RADIUS, ALARM_DEPOSIT_AMOUNT);

      // Move toward spider
      Vec2 spiderPos = em.GetComponent<CTransform>(spider).position;
      Vec2 toSpider = spiderPos - transform.position;
      float dist = toSpider.Length();

      if (dist > 0.001f) {
        wander.direction = toSpider / dist;
      }

      transform.velocity = wander.direction * speed.value;
    }
    else {
      // Spider gone, go back to wandering
      ant.state = AntState::WANDER;
      // WanderSystem will handle velocity
    }
  }

  void Update(EntityManager& em, SpatialGrid& grid, PheromoneGrid& pheromones,
    float deltaTime) {

    auto ants = em.GetEntitiesWithComponents(ANT | TRANSFORM | WANDER | SPEED);

    for (Entity e : ants) {
      auto& ant = em.GetComponent<CAnt>(e);

      // Deposit HOME pheromone while wandering (leaving nest)
      if (ant.state == AntState::WANDER) {
        auto& transform = em.GetComponent<CTransform>(e);
        pheromones.Deposit(PHEROMONE_HOME, transform.position, 2.0f);
      }

      switch (ant.state) {
        case AntState::WANDER:
          UpdateWander(e, em, grid, pheromones, deltaTime);
          break;

        case AntState::FOLLOW_TRAIL:
          break;

        case AntState::FORAGE:
          UpdateForage(e, em, grid, pheromones, deltaTime);
          break;

        case AntState::FLEE:
          UpdateFlee(e, em, grid, pheromones, deltaTime);
          break;

        case AntState::ATTACK:
          break;
      }
    }
  }

  void HandleCollisions(EntityManager& em, EventBuffer& events) {
    // Handle Ant <-> Food collisions (start dragging)
    for (const auto& collision : events.Get<AntFoodCollision>()) {
      Entity ant = collision.ant;
      Entity food = collision.food;

      // Verify entities still exist and ant is in WANDER state
      if (!em.HasComponents(ANT | TRANSFORM | DRAGGING, ant)) continue;
      if (!em.HasComponents(FOOD | DRAGGABLE, food)) continue;

      auto& antComp = em.GetComponent<CAnt>(ant);
      auto& dragging = em.GetComponent<CDragging>(ant);

      // Only pick up if wandering and not already dragging
      if (antComp.state != AntState::WANDER) continue;
      if (dragging.target != INVALID_ENTITY) continue;

      // Try to start dragging
      if (DragSystem::StartDragging(em, ant, food)) {
        // Clear target
        if (em.HasComponents(TARGET, ant)) {
          em.GetComponent<CTarget>(ant).entity = INVALID_ENTITY;
        }

        // Transition to FORAGE state
        antComp.state = AntState::FORAGE;
      }
    }

    // Handle Food <-> Colony collisions (deposit food)
    for (const auto& collision : events.Get<FoodColonyCollision>()) {
      Entity food = collision.food;
      Entity colony = collision.colony;

      // Verify entities exist
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

        auto& draggerDragging = em.GetComponent<CDragging>(dragger);
        draggerDragging.target = INVALID_ENTITY;

        auto& antComp = em.GetComponent<CAnt>(dragger);
        antComp.state = AntState::WANDER;
      }

      // Delete food
      em.DeleteEntity(food);
    }

    // Handle Ant <-> Spider collisions
    for (const auto& collision : events.Get<AntSpiderCollision>()) {
      Entity ant = collision.ant;
      Entity spider = collision.spider;

      // Verify entities exist
      if (!em.HasComponents(ANT | HEALTH | COMBAT, ant)) continue;
      if (!em.HasComponents(SPIDER | HEALTH | COMBAT, spider)) continue;

      auto& antComp = em.GetComponent<CAnt>(ant);
      auto& antHealth = em.GetComponent<CHealth>(ant);
      auto& antCombat = em.GetComponent<CCombat>(ant);
      auto& spiderHealth = em.GetComponent<CHealth>(spider);
      auto& spiderCombat = em.GetComponent<CCombat>(spider);

      // Spider deals damage to ant (if attack cooldown ready)
      if (spiderCombat.attackTimer <= 0.0f) {
        antHealth.current -= spiderCombat.attackDamage;
        spiderCombat.attackTimer = spiderCombat.attackCooldown;

        // Ant dies if health depleted
        if (antHealth.current <= 0.0f) {
          em.DeleteEntity(ant);
          continue;
        }
      }

      // Ant fights back if in ATTACK state
      if (antComp.state == AntState::ATTACK) {
        if (antCombat.attackTimer <= 0.0f) {
          spiderHealth.current -= antCombat.attackDamage;
          antCombat.attackTimer = antCombat.attackCooldown;

          // Spider dies if health depleted
          if (spiderHealth.current <= 0.0f) {
            em.DeleteEntity(spider);
            continue;
          }
        }
      }

      // Surviving ants that aren't already in combat enter FLEE state
      // (They'll switch to ATTACK if alarm pheromone is high enough)
      if (antComp.state != AntState::FLEE && antComp.state != AntState::ATTACK) {
        antComp.state = AntState::FLEE;
      }
    }
  }
}