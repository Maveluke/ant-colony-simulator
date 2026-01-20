#pragma once
#include "ECS/systems/AntContext.h"

namespace AntStates {
  namespace Wander {

    // WANDER: Look for food or threats, follow food trail if found
    // If escaping, skip pheromone logic
    // Transitions to: FLEE (threat), FOLLOW_TRAIL (found trail), or stays in WANDER
    void Update(AntContext& ctx);

  }
}
