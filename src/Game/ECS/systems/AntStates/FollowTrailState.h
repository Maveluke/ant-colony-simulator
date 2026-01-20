#pragma once
#include "ECS/systems/AntContext.h"

namespace AntStates {
  namespace FollowTrail {

    // FOLLOW_TRAIL: Following food pheromone trail to find food source
    // Transitions to: FLEE (spider), WANDER (trail lost), or stays following
    void Update(AntContext& ctx);

  }
}
