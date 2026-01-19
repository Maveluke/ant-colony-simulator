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
    // Get relevant entities
    auto ants = em.GetEntitiesWithComponents(ANT | TRANSFORM | CIRCLE_COLLIDER);
    auto foods = em.GetEntitiesWithComponents(FOOD | TRANSFORM | CIRCLE_COLLIDER);
    auto colonies = em.GetEntitiesWithComponents(COLONY | TRANSFORM | QUAD_RENDERER);
    auto spiders = em.GetEntitiesWithComponents(SPIDER | TRANSFORM | CIRCLE_COLLIDER);

    // Ant <-> Food collisions
    for (Entity ant : ants) {
      auto& antTransform = em.GetComponent<CTransform>(ant);
      auto& antCollider = em.GetComponent<CCircleCollider>(ant);
      auto& antDetection = em.GetComponent<CDetection>(ant);

      // Query nearby food using spatial grid
      auto nearby = grid.Query(antTransform.position, antDetection.radius);

      for (Entity other : nearby) {
        // Check if it's food
        if (!em.HasComponents(FOOD | CIRCLE_COLLIDER, other)) {
          continue;
        }

        auto& foodTransform = em.GetComponent<CTransform>(other);
        auto& foodCollider = em.GetComponent<CCircleCollider>(other);

        if (CirclesOverlap(antTransform.position, antCollider.radius,
          foodTransform.position, foodCollider.radius)) {
          events.Push(AntFoodCollision{ ant, other });
        }
      }
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

      // Query nearby ants
      auto nearby = grid.Query(spiderTransform.position,
        spiderCollider.radius + 20.0f);

      for (Entity other : nearby) {
        // Check if it's an ant
        if (!em.HasComponents(ANT | CIRCLE_COLLIDER, other)) {
          continue;
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
      }
    }
  }
}