#include "ECS/systems/helpers/StuckDetector.h"
#include "ECS/systems/helpers/RandomUtils.h"
#include <cmath>

namespace StuckDetector {

  bool CheckAndEscape(
    CWander& wander,
    const Vec2& currentPosition,
    float deltaTime,
    float distanceThreshold,
    float timeThreshold
  ) {
    // Check per-axis movement since last position update
    // This catches oscillating behavior where total distance is fine but no net progress
    Vec2 delta = currentPosition - wander.lastPosition;
    float absX = (delta.x < 0) ? -delta.x : delta.x;
    float absY = (delta.y < 0) ? -delta.y : delta.y;

    // Stuck if BOTH axes haven't moved enough
    if (absX < distanceThreshold && absY < distanceThreshold) {
      wander.stuckTimer += deltaTime;
    }
    else {
      // Moving fine, reset stuck timer and update last position
      wander.stuckTimer = 0.0f;
      wander.lastPosition = currentPosition;
    }

    // Are we stuck long enough to escape?
    if (wander.stuckTimer >= timeThreshold) {
      // Pick a random escape direction - turn significantly away from current direction
      float currentAngle = atan2(wander.direction.y, wander.direction.x);
      float escapeAngle = currentAngle + ESCAPE_ANGLE * (RandomUtils::Bool() ? 1.0f : -1.0f);

      // Add some extra randomness
      escapeAngle += RandomUtils::Float(-ESCAPE_ANGLE_VARIANCE, ESCAPE_ANGLE_VARIANCE);

      wander.direction.x = cos(escapeAngle);
      wander.direction.y = sin(escapeAngle);

      // Set escape timer to prevent direction from being overwritten
      wander.escapeTimer = ESCAPE_DURATION;

      // Reset stuck detection
      wander.stuckTimer = 0.0f;
      wander.lastPosition = currentPosition;

      return true;
    }

    return false;
  }
}