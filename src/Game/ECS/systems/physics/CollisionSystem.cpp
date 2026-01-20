#include "CollisionSystem.h"
#include "ECS/Components.h"
#include "ECS/Constants.h"

namespace CollisionSystem {

  // Check if two circles overlap
  static bool CirclesOverlap(const Vec2& pos1, float radius1,
    const Vec2& pos2, float radius2) {
    float combinedRadius = radius1 + radius2;
    return pos1.DistanceSquared(pos2) <= combinedRadius * combinedRadius;
  }

  // Check if circle overlaps with axis-aligned box 
  static bool CircleBoxOverlap(const Vec2& circlePos, float circleRadius,
    const Vec2& boxCenter, const Vec2& boxHalfSize) {
    // Find closest point on box to circle center
    float closestX = circlePos.x;
    float closestY = circlePos.y;

    if (circlePos.x < boxCenter.x - boxHalfSize.x) {
      closestX = boxCenter.x - boxHalfSize.x;
    }
    else if (circlePos.x > boxCenter.x + boxHalfSize.x) {
      closestX = boxCenter.x + boxHalfSize.x;
    }

    if (circlePos.y < boxCenter.y - boxHalfSize.y) {
      closestY = boxCenter.y - boxHalfSize.y;
    }
    else if (circlePos.y > boxCenter.y + boxHalfSize.y) {
      closestY = boxCenter.y + boxHalfSize.y;
    }

    // Check if closest point is within circle
    Vec2 closest(closestX, closestY);
    return circlePos.DistanceSquared(closest) <= circleRadius * circleRadius;
  }

  void Update(EntityManager& em, SpatialGrid& grid, EventBuffer& events) {
    // Get relevant entities from cache (much faster than GetEntitiesWithComponents)
    const auto& ants = em.GetAntsWithCollider();
    const auto& foods = em.GetFoods();
    const auto& colonies = em.GetColonies();
    const auto& spiders = em.GetSpiders();

    // Ant <-> Food collisions
    for (Entity ant : ants) {
      auto& antTransform = em.GetComponent<CTransform>(ant);
      auto& antCollider = em.GetComponent<CCircleCollider>(ant);
      auto& antDetection = em.GetComponent<CDetection>(ant);

      // Query nearby food using spatial grid (zero-allocation callback)
      grid.QueryEach(antTransform.position, antDetection.radius, [&](Entity other) {
        // Check if it's food
        if (!em.HasComponents(FOOD | CIRCLE_COLLIDER, other)) {
          return;
        }

        auto& foodTransform = em.GetComponent<CTransform>(other);
        auto& foodCollider = em.GetComponent<CCircleCollider>(other);

        if (CirclesOverlap(antTransform.position, antCollider.radius,
          foodTransform.position, foodCollider.radius)) {
          events.Push(AntFoodCollision{ ant, other });
        }
        });
    }

    // Ant <-> Colony collisions
    for (Entity ant : ants) {
      auto& antTransform = em.GetComponent<CTransform>(ant);
      auto& antCollider = em.GetComponent<CCircleCollider>(ant);

      for (Entity colony : colonies) {
        auto& colonyTransform = em.GetComponent<CTransform>(colony);
        auto& colonyRenderer = em.GetComponent<CQuadRenderer>(colony);
        Vec2 halfSize = colonyRenderer.size * 0.5f;

        if (CircleBoxOverlap(antTransform.position, antCollider.radius,
          colonyTransform.position, halfSize)) {
          events.Push(AntColonyCollision{ ant, colony });
        }
      }
    }

    // Food <-> Colony collisions
    for (Entity food : foods) {
      auto& foodTransform = em.GetComponent<CTransform>(food);
      auto& foodCollider = em.GetComponent<CCircleCollider>(food);

      for (Entity colony : colonies) {
        auto& colonyTransform = em.GetComponent<CTransform>(colony);
        auto& colonyRenderer = em.GetComponent<CQuadRenderer>(colony);
        Vec2 halfSize = colonyRenderer.size * 0.5f;

        if (CircleBoxOverlap(foodTransform.position, foodCollider.radius,
          colonyTransform.position, halfSize)) {
          events.Push(FoodColonyCollision{ food, colony });
        }
      }
    }

    // Ant <-> Spider collisions (with pushout)
    for (Entity spider : spiders) {
      auto& spiderTransform = em.GetComponent<CTransform>(spider);
      auto& spiderCollider = em.GetComponent<CCircleCollider>(spider);

      // Query nearby ants (zero-allocation callback)
      grid.QueryEach(spiderTransform.position, spiderCollider.radius + 20.0f, [&](Entity other) {
        // Check if it's an ant
        if (!em.HasComponents(ANT | CIRCLE_COLLIDER, other)) {
          return;
        }

        auto& antTransform = em.GetComponent<CTransform>(other);
        auto& antCollider = em.GetComponent<CCircleCollider>(other);

        if (CirclesOverlap(antTransform.position, antCollider.radius,
          spiderTransform.position, spiderCollider.radius)) {
          events.Push(AntSpiderCollision{ other, spider });

          // Pushout: move ant to edge of spider
          Vec2 pushDir = antTransform.position - spiderTransform.position;
          float dist = pushDir.Length();

          if (dist > 0.001f) {
            pushDir.Normalize();
            float minDist = spiderCollider.radius + antCollider.radius;
            antTransform.position = spiderTransform.position + pushDir * minDist;
          }
        }
        });
    }

    // Ant <-> Ant collisions (different teams only)
    for (size_t i = 0; i < ants.size(); ++i) {
      Entity ant1 = ants[i];
      auto& ant1Transform = em.GetComponent<CTransform>(ant1);
      auto& ant1Collider = em.GetComponent<CCircleCollider>(ant1);
      auto& ant1Comp = em.GetComponent<CAnt>(ant1);

      // Query nearby entities (zero-allocation callback)
      grid.QueryEach(ant1Transform.position, ant1Collider.radius + 10.0f, [&](Entity other) {
        // Skip self
        if (other == ant1) return;

        // Check if it's an ant
        if (!em.HasComponents(ANT | CIRCLE_COLLIDER, other)) {
          return;
        }

        auto& ant2Comp = em.GetComponent<CAnt>(other);

        // Only collide if different teams (and both have valid teams)
        if (ant1Comp.teamId == ant2Comp.teamId ||
          ant1Comp.teamId == TEAM_NONE ||
          ant2Comp.teamId == TEAM_NONE) {
          return;
        }

        auto& ant2Transform = em.GetComponent<CTransform>(other);
        auto& ant2Collider = em.GetComponent<CCircleCollider>(other);

        if (CirclesOverlap(ant1Transform.position, ant1Collider.radius,
          ant2Transform.position, ant2Collider.radius)) {
          events.Push(AntAntCollision{ ant1, other });
        }
        });
    }
  }
}