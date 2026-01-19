#include "ECS/systems/WanderSystem.h"
#include "ECS/Components.h"
#include "ECS/systems/physics/DragSystem.h"
#include <cstdlib>
#include <cmath>

namespace WanderSystem {

  // Helper: Generate random float between min and max
  static float RandomFloat(float min, float max) {
    return min + (rand() / (float)RAND_MAX) * (max - min);
  }

  // Helper: Slightly perturb current direction for smoother wandering
  static Vec2 PerturbDirection(const Vec2& currentDir) {
    float perturbAngle = RandomFloat(-1.0f, 1.0f);  // ~60 degree max turn
    float cosA = cos(perturbAngle);
    float sinA = sin(perturbAngle);

    // Rotate current direction by perturbAngle
    Vec2 newDir;
    newDir.x = currentDir.x * cosA - currentDir.y * sinA;
    newDir.y = currentDir.x * sinA + currentDir.y * cosA;

    return newDir.Normalized();
  }

  void Update(EntityManager& em, float deltaTime) {
    auto entities = em.GetEntitiesWithComponents(TRANSFORM | WANDER | SPEED);

    for (Entity e : entities) {
      // Skip entities that are currently dragging something
      // Their movement is controlled by DragSystem
      if (DragSystem::IsDragging(em, e)) {
        continue;
      }

      auto& transform = em.GetComponent<CTransform>(e);
      auto& wander = em.GetComponent<CWander>(e);
      auto& speed = em.GetComponent<CSpeed>(e);

      // Update wander timer
      wander.timer -= deltaTime;

      if (wander.timer <= 0.0f) {
        wander.direction = PerturbDirection(wander.direction);
        wander.timer = RandomFloat(0.5f, 2.0f);
      }

      // Set velocity from direction and speed
      transform.velocity = wander.direction * speed.value;
    }
  }

}