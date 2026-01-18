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
          break;

        case AntState::ATTACK:
          break;
      }
    }
  }
}