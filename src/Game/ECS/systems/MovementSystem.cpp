#include "MovementSystem.h"

namespace MovementSystem {
  bool Update(CTransform& transform, float deltaTime,
    float worldWidth, float worldHeight) {
    // Apply velocity to position
    transform.position = transform.position + transform.velocity * deltaTime;

    // Bounce off boundaries
    bool bounced = false;

    if (transform.position.x < 0.0f) {
      transform.position.x = 0.0f;
      transform.velocity.x = -transform.velocity.x;
      bounced = true;
    }
    else if (transform.position.x > worldWidth) {
      transform.position.x = worldWidth;
      transform.velocity.x = -transform.velocity.x;
      bounced = true;
    }

    if (transform.position.y < 0.0f) {
      transform.position.y = 0.0f;
      transform.velocity.y = -transform.velocity.y;
      bounced = true;
    }
    else if (transform.position.y > worldHeight) {
      transform.position.y = worldHeight;
      transform.velocity.y = -transform.velocity.y;
      bounced = true;
    }

    return bounced;
  }

}