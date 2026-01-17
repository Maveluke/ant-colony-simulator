#include "AntSystem.h"
#include "../Components.h"
#include "../Definition.h"
#include <cstdlib>
#include <cmath>

namespace AntSystem {

  // Helper: Generate random float between min and max
  static float RandomFloat(float min, float max) {
    return min + (rand() / (float)RAND_MAX) * (max - min);
  }

  // Helper: Generate random unit direction vector
  static Vec2 RandomDirection() {
    float angle = RandomFloat(0.0f, TWO_PI);
    return Vec2(cos(angle), sin(angle));
  }

  // Helper: Slightly perturb current direction for smoother wandering
  static Vec2 WanderDirection(const Vec2& currentDir) {
    // Add random offset to current direction, then normalize
    // This creates smoother turns rather than abrupt direction changes
    float perturbAngle = RandomFloat(-1.0f, 1.0f);
    float cosA = cos(perturbAngle);
    float sinA = sin(perturbAngle);

    // Rotate current direction by perturbAngle
    Vec2 newDir;
    newDir.x = currentDir.x * cosA - currentDir.y * sinA;
    newDir.y = currentDir.x * sinA + currentDir.y * cosA;

    return newDir.Normalized();
  }

  // Handle WANDER state: random movement
  static void UpdateWander(CAnt& ant, CTransform& transform, float deltaTime,
    float worldWidth, float worldHeight) {
    // Decrement timer
    ant.wanderTimer -= deltaTime;

    // Time to change direction?
    if (ant.wanderTimer <= 0.0f) {
      ant.direction = WanderDirection(ant.direction);
      ant.wanderTimer = RandomFloat(0.5f, 2.0f);
    }

    // Apply movement
    transform.velocity = ant.direction * ant.speed;
    transform.position = transform.position + transform.velocity * deltaTime;

    // Bounce off world boundaries
    bool bounced = false;

    if (transform.position.x < 0.0f) {
      transform.position.x = 0.0f;
      ant.direction.x = -ant.direction.x;
      bounced = true;
    }
    else if (transform.position.x > worldWidth) {
      transform.position.x = worldWidth;
      ant.direction.x = -ant.direction.x;
      bounced = true;
    }

    if (transform.position.y < 0.0f) {
      transform.position.y = 0.0f;
      ant.direction.y = -ant.direction.y;
      bounced = true;
    }
    else if (transform.position.y > worldHeight) {
      transform.position.y = worldHeight;
      ant.direction.y = -ant.direction.y;
      bounced = true;
    }

    // Reset timer on bounce so ant doesn't immediately turn away from wall
    if (bounced) {
      ant.wanderTimer = RandomFloat(0.3f, 0.8f);
    }
  }

  void Update(EntityManager& em, SpatialGrid& grid, float deltaTime,
    float worldWidth, float worldHeight) {

    // Get all entities with ANT and TRANSFORM components
    auto ants = em.GetEntitiesWithComponents(ANT | TRANSFORM);

    for (Entity e : ants) {
      auto& ant = em.GetComponent<CAnt>(e);
      auto& transform = em.GetComponent<CTransform>(e);

      switch (ant.state) {
        case AntState::WANDER:
          UpdateWander(ant, transform, deltaTime, worldWidth, worldHeight);
          break;

        case AntState::FOLLOW_TRAIL:
          break;

        case AntState::FORAGE:
          break;

        case AntState::FLEE:
          break;

        case AntState::ATTACK:
          break;
      }
    }
  }

}