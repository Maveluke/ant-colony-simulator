#pragma once
#include "ECS/Components.h"
#include "Vec2.h"

namespace StuckDetector {

  // For individual ants (more sensitive)
  constexpr float ANT_DISTANCE_THRESHOLD = 8.0f;
  constexpr float ANT_TIME_THRESHOLD = 0.3f;

  // For drag groups (more patient - groups move slower)
  constexpr float DRAG_DISTANCE_THRESHOLD = 2.0f;
  constexpr float DRAG_TIME_THRESHOLD = 0.6f;

  // Shared escape behavior
  constexpr float ESCAPE_ANGLE = PI * 0.75f;     // Turn 135 degrees
  constexpr float ESCAPE_DURATION = 0.5f;        // How long to keep escaping
  constexpr float ESCAPE_ANGLE_VARIANCE = 0.5f;  // Random variance in escape angle

  // =============================================================================
  // Stuck Detection Functions
  // =============================================================================

  // Check if entity is stuck and handle escape
  // Updates wander.lastPosition, wander.stuckTimer, wander.escapeTimer, wander.direction
  //
  // Parameters:
  //   wander            - Wander component to update
  //   currentPosition   - Position to track (entity's position or food's position for groups)
  //   deltaTime         - Frame delta time
  //   distanceThreshold - Min distance to move per check to not be "stuck"
  //   timeThreshold     - How long to be stuck before escaping
  //
  // Returns:
  //   true if entity was stuck and escape direction was set
  bool CheckAndEscape(
    CWander& wander,
    const Vec2& currentPosition,
    float deltaTime,
    float distanceThreshold,
    float timeThreshold
  );

  // Convenience: Check for individual ants (uses ANT_* thresholds)
  inline bool CheckAndEscapeAnt(CWander& wander, const Vec2& position, float deltaTime) {
    return CheckAndEscape(wander, position, deltaTime,
      ANT_DISTANCE_THRESHOLD, ANT_TIME_THRESHOLD);
  }

  // Convenience: Check for drag groups (uses DRAG_* thresholds)
  inline bool CheckAndEscapeDragGroup(CWander& wander, const Vec2& foodPosition, float deltaTime) {
    return CheckAndEscape(wander, foodPosition, deltaTime,
      DRAG_DISTANCE_THRESHOLD, DRAG_TIME_THRESHOLD);
  }
}