#include "ECS/systems/physics/DragSystem.h"
#include "ECS/Components.h"
#include "ECS/Constants.h"
#include "ECS/Definition.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace DragSystem {

  constexpr float BASE_DRAG_SPEED = 50.0f;
  constexpr float MIN_EFFICIENCY = 0.2f;  // Minimum speed multiplier (even 1 ant can slowly drag)

  // Monte Carlo sampling constants (shared with AntSystem concept)
  constexpr int NUM_PHEROMONE_SAMPLES = 5;  // Reduced from 8 for performance
  constexpr float SAMPLE_MIN_DISTANCE = 16.0f;
  constexpr float SAMPLE_MAX_DISTANCE = 48.0f;
  constexpr float PHEROMONE_SAMPLE_INTERVAL = 1.0f;
  constexpr float NARROW_CONE_ANGLE = 0.785f;  // ~45 degrees - focused toward home

  // Stuck detection constants
  constexpr float STUCK_DISTANCE_THRESHOLD = 2.0f;   // Min distance to move per check
  constexpr float STUCK_TIME_THRESHOLD = 0.6f;       // Time before considered stuck
  constexpr float STUCK_ESCAPE_ANGLE = PI * 0.75f;   // Turn 135 degrees when stuck
  constexpr float ESCAPE_DURATION = 0.5f;            // How long to keep escaping direction
  constexpr float DIRECTION_LERP_FACTOR = 0.5f;      // Direction smoothing

  // Helper: Generate random float between min and max
  static float RandomFloat(float min, float max) {
    return min + (rand() / static_cast<float>(RAND_MAX)) * (max - min);
  }

  // Monte Carlo pheromone sampling from a position using team-specific home pheromones
  // Returns direction toward the best pheromone sample, or zero vector if none found
  static Vec2 SampleBestHomeDirection(
    ColonyPheromoneManager& colonyPheromones,
    TeamId team,
    const Vec2& position,
    const Vec2& currentDirection,
    const Vec2& homePosition,
    float coneAngle
  ) {
    float bestIntensity = 0.0f;
    Vec2 bestPosition = position;

    float baseAngle = atan2(currentDirection.y, currentDirection.x);

    for (int i = 0; i < NUM_PHEROMONE_SAMPLES; i++) {
      float sampleAngle = baseAngle + RandomFloat(-coneAngle, coneAngle);
      float sampleDistance = RandomFloat(SAMPLE_MIN_DISTANCE, SAMPLE_MAX_DISTANCE);

      Vec2 samplePos;
      samplePos.x = position.x + cos(sampleAngle) * sampleDistance;
      samplePos.y = position.y + sin(sampleAngle) * sampleDistance;

      float intensity = colonyPheromones.GetHomeIntensity(team, samplePos);

      if (intensity > bestIntensity) {
        bestIntensity = intensity;
        bestPosition = samplePos;
      }
    }

    if (bestIntensity > 0.0f) {
      Vec2 direction = bestPosition - position;
      Vec2 towardHome = homePosition - position;
      // Blend direction toward home to keep group on track
      direction += towardHome * 0.03f;
      float length = direction.Length();
      if (length > 0.001f) {
        return direction / length;
      }
    }

    return Vec2(0.0f, 0.0f);
  }

  float GetEfficiency(const CDraggable& draggable) {
    if (draggable.draggerCount == 0) return 0.0f;
    float efficiency = static_cast<float>(draggable.draggerCount) / draggable.weight;
    return std::clamp(efficiency, MIN_EFFICIENCY, 1.0f);
  }

  bool StartDragging(EntityManager& em, Entity dragger, Entity target) {
    // Validate entities
    if (!em.HasComponents(DRAGGING | ANT, dragger)) return false;
    if (!em.HasComponents(DRAGGABLE | TRANSFORM, target)) return false;

    auto& dragging = em.GetComponent<CDragging>(dragger);
    auto& draggable = em.GetComponent<CDraggable>(target);
    auto& draggerAnt = em.GetComponent<CAnt>(dragger);

    // Already dragging something?
    if (dragging.target != INVALID_ENTITY) {
      return false;
    }

    // Target at max draggers?
    if (draggable.draggerCount >= draggable.maxDraggers) {
      return false;
    }

    // If food already has draggers, check team compatibility
    if (draggable.draggerCount > 0) {
      Entity firstDragger = draggable.draggers[0];
      if (em.HasComponents(ANT, firstDragger)) {
        auto& firstDraggerAnt = em.GetComponent<CAnt>(firstDragger);
        // Only allow same team to help drag
        if (draggerAnt.teamId != firstDraggerAnt.teamId) {
          return false;
        }
      }
    }

    dragging.target = target;
    draggable.draggers[draggable.draggerCount] = dragger;
    draggable.draggerCount++;

    return true;
  }

  void StopDragging(EntityManager& em, Entity dragger) {
    if (!em.HasComponents(DRAGGING, dragger)) return;

    auto& dragging = em.GetComponent<CDragging>(dragger);
    Entity target = dragging.target;

    if (target == INVALID_ENTITY) return;

    // Remove from target's dragger list
    if (em.HasComponents(DRAGGABLE, target)) {
      auto& draggable = em.GetComponent<CDraggable>(target);

      for (int i = 0; i < draggable.draggerCount; i++) {
        if (draggable.draggers[i] == dragger) {
          // Swap with last and decrement count
          draggable.draggers[i] = draggable.draggers[draggable.draggerCount - 1];
          draggable.draggers[draggable.draggerCount - 1] = INVALID_ENTITY;
          draggable.draggerCount--;
          break;
        }
      }
    }

    dragging.target = INVALID_ENTITY;
  }

  bool IsDragging(EntityManager& em, Entity entity) {
    if (!em.HasComponents(DRAGGING, entity)) return false;
    return em.GetComponent<CDragging>(entity).target != INVALID_ENTITY;
  }

  void Update(EntityManager& em, PheromoneGrid& pheromones,
    ColonyPheromoneManager& colonyPheromones, float deltaTime) {
    auto draggables = em.GetEntitiesWithComponents(DRAGGABLE | TRANSFORM);

    for (Entity e : draggables) {
      auto& draggable = em.GetComponent<CDraggable>(e);
      auto& foodTransform = em.GetComponent<CTransform>(e);

      // No draggers? Entity doesn't move
      if (draggable.draggerCount == 0) {
        foodTransform.velocity = Vec2(0.0f, 0.0f);
        continue;
      }

      // Get the first dragger to use its wander component for timing and direction state
      Entity firstDragger = draggable.draggers[0];
      if (!em.HasComponents(TRANSFORM | WANDER | ANT, firstDragger)) {
        foodTransform.velocity = Vec2(0.0f, 0.0f);
        continue;
      }

      auto& leaderWander = em.GetComponent<CWander>(firstDragger);
      auto& leaderAnt = em.GetComponent<CAnt>(firstDragger);

      // Get the leader's home colony position for fallback navigation
      Vec2 homeColonyPos = foodTransform.position;  // Default fallback
      if (leaderAnt.homeColony != INVALID_ENTITY &&
        em.HasComponents(TRANSFORM, leaderAnt.homeColony)) {
        homeColonyPos = em.GetComponent<CTransform>(leaderAnt.homeColony).position;
      }

      // Use the leader's pheromone timer for the whole group
      leaderWander.pheromoneTimer -= deltaTime;

      // Count down escape timer
      if (leaderWander.escapeTimer > 0.0f) {
        leaderWander.escapeTimer -= deltaTime;
      }

      Vec2 groupDirection = leaderWander.direction;

      // Only sample pheromones if not escaping
      if (leaderWander.escapeTimer <= 0.0f && leaderWander.pheromoneTimer <= 0.0f) {
        leaderWander.pheromoneTimer = PHEROMONE_SAMPLE_INTERVAL;

        // Sample team-specific HOME pheromone from the FOOD's position
        // This way all ants agree on the same direction and follow their own colony's trail
        Vec2 homeDir = SampleBestHomeDirection(
          colonyPheromones, leaderAnt.teamId, foodTransform.position,
          leaderWander.direction, homeColonyPos, NARROW_CONE_ANGLE
        );

        if (homeDir.LengthSquared() > 0.0001f) {
          // Smooth direction change to prevent jittery movement
          groupDirection = leaderWander.direction.Lerp(homeDir, DIRECTION_LERP_FACTOR);
          groupDirection.Normalize();
          leaderWander.direction = groupDirection;  // Update leader's direction for next sample
        }
        // Else: keep current direction
      }

      // Stuck detection for drag groups - check per-axis movement
      // This catches oscillating behavior where total distance is fine but no net progress
      Vec2 delta = foodTransform.position - leaderWander.lastPosition;
      float absX = (delta.x < 0) ? -delta.x : delta.x;
      float absY = (delta.y < 0) ? -delta.y : delta.y;

      // Stuck if BOTH axes haven't moved enough
      if (absX < STUCK_DISTANCE_THRESHOLD && absY < STUCK_DISTANCE_THRESHOLD) {
        leaderWander.stuckTimer += deltaTime;
      }
      else {
        leaderWander.stuckTimer = 0.0f;
        leaderWander.lastPosition = foodTransform.position;
      }

      // If stuck, pick escape direction and set escape timer
      if (leaderWander.stuckTimer >= STUCK_TIME_THRESHOLD) {
        float currentAngle = atan2(groupDirection.y, groupDirection.x);
        float escapeAngle = currentAngle + STUCK_ESCAPE_ANGLE * (RandomFloat(0, 1) > 0.5f ? 1.0f : -1.0f);
        escapeAngle += RandomFloat(-0.5f, 0.5f);

        groupDirection.x = cos(escapeAngle);
        groupDirection.y = sin(escapeAngle);
        leaderWander.direction = groupDirection;

        // Set escape timer to prevent pheromone sampling from overriding
        leaderWander.escapeTimer = ESCAPE_DURATION;

        leaderWander.stuckTimer = 0.0f;
        leaderWander.lastPosition = foodTransform.position;
      }

      // Normalize direction
      if (groupDirection.LengthSquared() > 0.0001f) {
        groupDirection.Normalize();
      }
      else {
        foodTransform.velocity = Vec2(0.0f, 0.0f);
        // Zero out all dragger velocities
        for (int i = 0; i < draggable.draggerCount; i++) {
          Entity dragger = draggable.draggers[i];
          if (em.HasComponents(TRANSFORM, dragger)) {
            em.GetComponent<CTransform>(dragger).velocity = Vec2(0.0f, 0.0f);
          }
        }
        continue;
      }

      // Calculate shared speed based on efficiency
      float efficiency = GetEfficiency(draggable);
      float speed = BASE_DRAG_SPEED * efficiency;

      // Shared velocity for the entire group
      Vec2 groupVelocity = groupDirection * speed;

      // Apply to food
      foodTransform.velocity = groupVelocity;

      // Apply same velocity and direction to ALL draggers (they move as a unit)
      for (int i = 0; i < draggable.draggerCount; i++) {
        Entity dragger = draggable.draggers[i];

        if (!em.HasComponents(TRANSFORM, dragger)) continue;

        auto& draggerTransform = em.GetComponent<CTransform>(dragger);
        draggerTransform.velocity = groupVelocity;

        // Sync all draggers' wander direction so they're aligned
        if (em.HasComponents(WANDER, dragger)) {
          em.GetComponent<CWander>(dragger).direction = groupDirection;
        }
      }
    }
  }

}