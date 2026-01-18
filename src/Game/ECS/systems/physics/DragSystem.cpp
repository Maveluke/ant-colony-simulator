#include "DragSystem.h"
#include "../Components.h"
#include "../Constants.h"
#include "../Definition.h"
#include <algorithm>
#include <cmath>

namespace DragSystem {

  constexpr float BASE_DRAG_SPEED = 40.0f;
  constexpr float DRAGGER_SNAP_SPEED = 5.0f;  // Higher = snappier following

  bool StartDragging(EntityManager& em, Entity dragger, Entity target) {
    // Validate entities
    if (!em.HasComponents(DRAGGING, dragger)) return false;
    if (!em.HasComponents(DRAGGABLE | TRANSFORM, target)) return false;

    auto& dragging = em.GetComponent<CDragging>(dragger);
    auto& draggable = em.GetComponent<CDraggable>(target);

    // Already dragging something?
    if (dragging.target != INVALID_ENTITY) {
      return false;
    }

    // Target at max draggers?
    if (draggable.draggerCount >= draggable.maxDraggers) {
      return false;
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

  void Update(EntityManager& em, float deltaTime) {
    auto draggables = em.GetEntitiesWithComponents(DRAGGABLE | TRANSFORM);

    for (Entity e : draggables) {
      auto& draggable = em.GetComponent<CDraggable>(e);
      auto& transform = em.GetComponent<CTransform>(e);

      // No draggers? Entity doesn't move 
      if (draggable.draggerCount == 0) {
        transform.velocity = Vec2(0.0f, 0.0f);
        continue;
      }

      // Calculate average pull direction from all draggers
      Vec2 totalPull(0.0f, 0.0f);
      int validDraggers = 0;

      for (int i = 0; i < draggable.draggerCount; i++) {
        Entity dragger = draggable.draggers[i];

        if (!em.HasComponents(TRANSFORM, dragger)) continue;

        auto& draggerTransform = em.GetComponent<CTransform>(dragger);

        // Pull direction is from draggable toward dragger's movement direction
        // (dragger is trying to pull the object where it wants to go)
        Vec2 draggerVel = draggerTransform.velocity;
        if (draggerVel.LengthSquared() > 0.0001f) {
          totalPull += draggerVel.Normalized();
          validDraggers++;
        }
      }

      if (validDraggers == 0) {
        transform.velocity = Vec2(0.0f, 0.0f);
        continue;
      }

      // Average direction
      Vec2 pullDir = totalPull / static_cast<float>(validDraggers);
      if (pullDir.LengthSquared() > 0.0001f) {
        pullDir.Normalize();
      }

      // Speed scales with dragger count vs weight
      float efficiency = static_cast<float>(draggable.draggerCount) / draggable.weight;
      float speed = BASE_DRAG_SPEED * std::min(efficiency, 1.0f);

      transform.velocity = pullDir * speed;

      // Keep draggers near the draggable (they move with it)
      for (int i = 0; i < draggable.draggerCount; i++) {
        Entity dragger = draggable.draggers[i];
        if (!em.HasComponents(TRANSFORM | CIRCLE_COLLIDER, dragger)) continue;

        auto& draggerTransform = em.GetComponent<CTransform>(dragger);
        auto& draggerCollider = em.GetComponent<CCircleCollider>(dragger);

        // Get draggable's collider radius
        float draggableRadius = 0.0f;
        if (em.HasComponents(CIRCLE_COLLIDER, e)) {
          draggableRadius = em.GetComponent<CCircleCollider>(e).radius;
        }

        // Calculate offset from draggable center (spread draggers around)
        float angle = (static_cast<float>(i) / draggable.draggerCount) * TWO_PI;
        float offsetDist = draggableRadius + draggerCollider.radius;
        Vec2 offset(cos(angle) * offsetDist, sin(angle) * offsetDist);

        // Lerp dragger position toward target offset position
        Vec2 targetPos = transform.position + offset;
        draggerTransform.position = draggerTransform.position.Lerp(targetPos, DRAGGER_SNAP_SPEED * deltaTime);
      }
    }
  }

}