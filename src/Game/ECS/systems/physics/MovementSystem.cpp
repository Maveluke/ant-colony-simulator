#include "MovementSystem.h"
#include "ECS/Components.h"

namespace MovementSystem {

  void Update(EntityManager& em, float deltaTime,
    float worldWidth, float worldHeight) {

    auto entities = em.GetEntitiesWithComponents(TRANSFORM);

    for (Entity e : entities) {
      auto& transform = em.GetComponent<CTransform>(e);

      // Apply velocity to position
      transform.position += transform.velocity * deltaTime;

      // Bounce off boundaries
      if (transform.position.x < 0.0f) {
        transform.position.x = 0.0f;
        transform.velocity.x = -transform.velocity.x;
      }
      else if (transform.position.x > worldWidth) {
        transform.position.x = worldWidth;
        transform.velocity.x = -transform.velocity.x;
      }

      if (transform.position.y < 0.0f) {
        transform.position.y = 0.0f;
        transform.velocity.y = -transform.velocity.y;
      }
      else if (transform.position.y > worldHeight) {
        transform.position.y = worldHeight;
        transform.velocity.y = -transform.velocity.y;
      }
    }
  }

}