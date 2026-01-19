#include "ECS/systems/AntSystem.h"
#include "ECS/Components.h"
#include "ECS/Constants.h"
#include <cstdlib>
#include <cmath>

namespace AntSystem {

  // Constants
  constexpr float ALARM_DEPOSIT_AMOUNT = 3.0f;
  constexpr float ALARM_DEPOSIT_RADIUS = 30.0f;
  constexpr float ALARM_ATTACK_THRESHOLD = 20.0f;
  constexpr float FOOD_TRAIL_THRESHOLD = 0.0f;

  // Monte Carlo sampling constants
  constexpr int NUM_PHEROMONE_SAMPLES = 8;           // Samples per decision
  constexpr float SAMPLE_MIN_DISTANCE = 16.0f;       // 1 cell
  constexpr float SAMPLE_MAX_DISTANCE = 48.0f;       // 3 cells
  constexpr float PHEROMONE_SAMPLE_INTERVAL = 1.0f; // Seconds between samples
  constexpr float WIDE_CONE_ANGLE = 3 * PI / 4;           // ~135 degrees (3*PI/4)
  constexpr float NARROW_CONE_ANGLE = PI / 4;        // ~45 degrees (PI/4)

  // Stuck detection constants
  constexpr float STUCK_DISTANCE_THRESHOLD = 2.0f;   // Min distance to move per check
  constexpr float STUCK_CHECK_INTERVAL = 0.3f;       // How often to check if stuck
  constexpr float STUCK_TIME_THRESHOLD = 0.6f;       // Time before considered stuck
  constexpr float STUCK_ESCAPE_ANGLE = PI * 0.75f;   // Turn 135 degrees when stuck
  constexpr float ESCAPE_DURATION = 0.5f;            // How long to keep escaping direction

  // Helper: Generate random float between min and max
  static float RandomFloat(float min, float max) {
    return min + (rand() / static_cast<float>(RAND_MAX)) * (max - min);
  }

  // Helper: Check if entity is stuck and handle escape
  // Returns true if entity was stuck and direction was changed
  static bool CheckAndHandleStuck(CTransform& transform, CWander& wander, float deltaTime) {
    // Check distance moved since last position update
    float distMoved = (transform.position - wander.lastPosition).Length();

    // Update stuck timer based on movement
    if (distMoved < STUCK_DISTANCE_THRESHOLD) {
      wander.stuckTimer += deltaTime;
    }
    else {
      // Moving fine, reset stuck timer and update last position
      wander.stuckTimer = 0.0f;
      wander.lastPosition = transform.position;
    }

    // Are we stuck long enough to escape?
    if (wander.stuckTimer >= STUCK_TIME_THRESHOLD) {
      // Pick a random escape direction - turn significantly away from current direction
      float currentAngle = atan2(wander.direction.y, wander.direction.x);
      float escapeAngle = currentAngle + STUCK_ESCAPE_ANGLE * (RandomFloat(0, 1) > 0.5f ? 1.0f : -1.0f);

      // Add some extra randomness
      escapeAngle += RandomFloat(-0.5f, 0.5f);

      wander.direction.x = cos(escapeAngle);
      wander.direction.y = sin(escapeAngle);

      // Set escape timer to prevent direction from being overwritten
      wander.escapeTimer = ESCAPE_DURATION;

      // Reset stuck detection
      wander.stuckTimer = 0.0f;
      wander.lastPosition = transform.position;

      return true;
    }

    return false;
  }

  static void ChangeState(CAnt& ant, AntState newState) {
    ant.state = newState;
  }

  // Monte Carlo pheromone sampling
  // Samples random positions in a cone and returns direction to the best one
  // coneAngle: half-angle of the cone in radians (e.g., PI/2 = ±90°)
  static Vec2 SampleBestPheromoneDirection(
    const PheromoneGrid& pheromones,
    PheromoneType type,
    const Vec2& position,
    const Vec2& currentDirection,
    float coneAngle = WIDE_CONE_ANGLE
  ) {
    float bestIntensity = 0.0f;
    Vec2 bestPosition = position;

    // Get current direction angle
    float baseAngle = atan2(currentDirection.y, currentDirection.x);

    for (int i = 0; i < NUM_PHEROMONE_SAMPLES; i++) {
      // Random angle within cone
      float sampleAngle = baseAngle + RandomFloat(-coneAngle, coneAngle);

      // Random distance
      float sampleDistance = RandomFloat(SAMPLE_MIN_DISTANCE, SAMPLE_MAX_DISTANCE);

      // Calculate sample position
      Vec2 samplePos;
      samplePos.x = position.x + cos(sampleAngle) * sampleDistance;
      samplePos.y = position.y + sin(sampleAngle) * sampleDistance;

      // Read pheromone intensity at sample position
      float intensity = pheromones.GetIntensity(type, samplePos);

      // Track the best sample
      if (intensity > bestIntensity) {
        bestIntensity = intensity;
        bestPosition = samplePos;
      }
    }

    // If we found pheromones, return direction toward best sample
    if (bestIntensity > 0.0f) {
      Vec2 direction = bestPosition - position;
      float length = direction.Length();
      if (length > 0.001f) {
        return direction / length; // Normalized
      }
    }

    // No pheromones found - return zero vector
    return Vec2(0.0f, 0.0f);
  }

  // Helper: Check if food can accept more draggers
  static bool CanDragFood(EntityManager& em, Entity food) {
    if (!em.HasComponents(DRAGGABLE, food)) return false;
    auto& draggable = em.GetComponent<CDraggable>(food);
    return draggable.draggerCount < draggable.maxDraggers;
  }

  // Helper: Find nearest food that can be picked up (not at max draggers)
  static Entity FindAvailableFood(EntityManager& em, SpatialGrid& grid,
    const Vec2& position, float radius) {

    auto nearby = grid.Query(position, radius);

    float nearestDistSq = radius * radius + 1.0f;
    Entity nearestFood = INVALID_ENTITY;

    for (Entity e : nearby) {
      if (!em.HasComponents(FOOD | TRANSFORM | CIRCLE_COLLIDER, e)) continue;
      if (!CanDragFood(em, e)) continue;  // Skip full food

      const Vec2& foodPos = em.GetComponent<CTransform>(e).position;
      float distSq = position.DistanceSquared(foodPos);

      if (distSq < nearestDistSq) {
        nearestDistSq = distSq;
        nearestFood = e;
      }
    }

    return nearestFood;
  }

  // Handle WANDER state - look for food, override direction if found
  // WanderSystem handles default wandering behavior
  static void UpdateWander(Entity e, EntityManager& em, SpatialGrid& grid,
    PheromoneGrid& pheromones, float deltaTime) {
    auto& ant = em.GetComponent<CAnt>(e);
    auto& transform = em.GetComponent<CTransform>(e);
    auto& wander = em.GetComponent<CWander>(e);
    auto& speed = em.GetComponent<CSpeed>(e);
    auto& detection = em.GetComponent<CDetection>(e);
    auto& target = em.GetComponent<CTarget>(e);

    // Count down escape timer - if escaping, skip pheromone detection
    if (wander.escapeTimer > 0.0f) {
      wander.escapeTimer -= deltaTime;
      // Let WanderSystem handle movement with current escape direction
      target.entity = INVALID_ENTITY;
      return;
    }

    // Look for nearby food that can be picked up
    Entity nearestFood = FindAvailableFood(em, grid, transform.position, detection.radius);

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
      return;
    }

    // No food nearby. Check for FOOD pheromone trail
    Vec2 foodPheromone = SampleBestPheromoneDirection(pheromones, PHEROMONE_FOOD, transform.position, wander.direction, WIDE_CONE_ANGLE);
    if (foodPheromone.LengthSquared() > 0.0001f) {
      // Follow the trail!
      ChangeState(ant, AntState::FOLLOW_TRAIL);
      return;
    }

    // No food or trail nearby, let WanderSystem handle movement
    target.entity = INVALID_ENTITY;
  }

  static void UpdateFollowTrail(Entity e, EntityManager& em, SpatialGrid& grid,
    PheromoneGrid& pheromones, float deltaTime) {
    auto& ant = em.GetComponent<CAnt>(e);
    auto& transform = em.GetComponent<CTransform>(e);
    auto& wander = em.GetComponent<CWander>(e);
    auto& speed = em.GetComponent<CSpeed>(e);
    auto& detection = em.GetComponent<CDetection>(e);
    auto& target = em.GetComponent<CTarget>(e);

    // First check for immediate threats (spiders)
    Entity spider = grid.QueryNearest(transform.position, detection.radius,
      SPIDER | TRANSFORM, em);
    if (spider != INVALID_ENTITY) {
      ChangeState(ant, AntState::FLEE);
      return;
    }

    // Check for actual food that can be picked up
    Entity nearestFood = FindAvailableFood(em, grid, transform.position, detection.radius);

    if (nearestFood != INVALID_ENTITY) {
      // Found food! Target it to trigger dragging next frame
      target.entity = nearestFood;
      Vec2 foodPos = em.GetComponent<CTransform>(nearestFood).position;
      Vec2 toFood = foodPos - transform.position;
      float dist = toFood.Length();
      if (dist > 0.001f) {
        wander.direction = toFood / dist;
        transform.velocity = wander.direction * speed.value;
      }
      return;
    }

    // Check pheromone sampling timer
    wander.pheromoneTimer -= deltaTime;

    // Count down escape timer
    if (wander.escapeTimer > 0.0f) {
      wander.escapeTimer -= deltaTime;
      // While escaping, just use current direction - skip pheromone sampling
      transform.velocity = wander.direction * speed.value;
      return;
    }

    float currentFoodPheromone = pheromones.GetIntensity(PHEROMONE_FOOD, transform.position);

    if (currentFoodPheromone < FOOD_TRAIL_THRESHOLD) {
      // Trail too weak, back to wandering
      ChangeState(ant, AntState::WANDER);
      target.entity = INVALID_ENTITY;
      return;
    }

    // Only resample direction when timer expires
    if (wander.pheromoneTimer <= 0.0f) {
      wander.pheromoneTimer = PHEROMONE_SAMPLE_INTERVAL;

      // Monte Carlo sample for FOOD pheromone (wide cone, exploring)
      Vec2 trailDir = SampleBestPheromoneDirection(
        pheromones, PHEROMONE_FOOD, transform.position,
        wander.direction, WIDE_CONE_ANGLE
      );

      if (trailDir.LengthSquared() > 0.0001f) {
        wander.direction = trailDir;
      }
      // If no direction found, keep current direction
    }

    // Apply movement
    transform.velocity = wander.direction * speed.value;
  }

  // Handle FORAGE state - returning home with food
  // DragSystem handles direction via Monte Carlo sampling from food position
  // This function only deposits FOOD pheromone and overrides direction if colony is visible
  static void UpdateForage(Entity e, EntityManager& em, SpatialGrid& grid,
    PheromoneGrid& pheromones, float deltaTime) {
    auto& transform = em.GetComponent<CTransform>(e);
    auto& wander = em.GetComponent<CWander>(e);
    auto& detection = em.GetComponent<CDetection>(e);

    // Deposit FOOD pheromone as the ants walk
    pheromones.Deposit(PHEROMONE_FOOD, transform.position, 10.0f);

    // If escaping, don't override direction - let escape continue
    if (wander.escapeTimer > 0.0f) {
      wander.escapeTimer -= deltaTime;
      return;
    }

    // Look for colony - if visible, override direction toward it
    Entity colony = grid.QueryNearest(transform.position, detection.radius,
      COLONY | TRANSFORM, em);

    if (colony != INVALID_ENTITY) {
      // Found colony! Set direction toward it (overrides DragSystem's direction)
      Vec2 colonyPos = em.GetComponent<CTransform>(colony).position;
      Vec2 toColony = colonyPos - transform.position;
      float dist = toColony.Length();

      if (dist > 0.001f) {
        wander.direction = toColony / dist;
      }
    }
    // Else: DragSystem handles direction via Monte Carlo sampling from food position

    // NOTE: We do NOT set velocity here!
    // DragSystem will read wander.direction and apply unified velocity to the drag group
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
        ChangeState(ant, AntState::ATTACK);
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
      ChangeState(ant, AntState::WANDER);
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
      ChangeState(ant, AntState::WANDER);
      // WanderSystem will handle velocity
    }
  }

  void Update(EntityManager& em, SpatialGrid& grid, PheromoneGrid& pheromones,
    float deltaTime) {

    auto ants = em.GetEntitiesWithComponents(ANT | TRANSFORM | WANDER | SPEED);
    auto colonies = em.GetEntitiesWithComponents(COLONY | TRANSFORM);
    Entity colony = colonies.empty() ? INVALID_ENTITY : colonies[0];

    // Deposit HOME pheromone around colony (permanent beacon)
    // if (colony != INVALID_ENTITY) {
    //   auto& colonyTransform = em.GetComponent<CTransform>(colony);
    //   pheromones.DepositRadius(PHEROMONE_HOME, colonyTransform.position, 50.0f, 10.0f);
    // }

    for (Entity e : ants) {
      auto& ant = em.GetComponent<CAnt>(e);
      auto& antCombat = em.GetComponent<CCombat>(e);

      // Count down attack timer
      if (antCombat.attackTimer > 0.0f) {
        antCombat.attackTimer -= deltaTime;
      }

      // Deposit HOME pheromone while wandering (leaving nest)
      if (ant.state == AntState::WANDER || ant.state == AntState::FOLLOW_TRAIL) {
        auto& transform = em.GetComponent<CTransform>(e);
        pheromones.Deposit(PHEROMONE_HOME, transform.position, 10.0f);
      }

      // Check if ant is stuck and handle escape (skip for dragging ants - handled in DragSystem)
      if (!DragSystem::IsDragging(em, e)) {
        auto& transform = em.GetComponent<CTransform>(e);
        auto& wander = em.GetComponent<CWander>(e);
        CheckAndHandleStuck(transform, wander, deltaTime);
      }

      switch (ant.state) {
        case AntState::WANDER:
          UpdateWander(e, em, grid, pheromones, deltaTime);
          break;

        case AntState::FOLLOW_TRAIL:
          UpdateFollowTrail(e, em, grid, pheromones, deltaTime);
          break;

        case AntState::FORAGE:
          UpdateForage(e, em, grid, pheromones, deltaTime);
          break;

        case AntState::FLEE:
          UpdateFlee(e, em, grid, pheromones, deltaTime);
          break;

        case AntState::ATTACK:
          UpdateAttack(e, em, grid, pheromones, deltaTime);
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
      if (antComp.state != AntState::WANDER && antComp.state != AntState::FOLLOW_TRAIL) continue;
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