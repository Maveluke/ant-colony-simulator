#pragma once
#include "ECS/systems/AntContext.h"

namespace AntStates {
  namespace Flee {

    // FLEE: Running away from spider or alarm source
    // Deposits alarm pheromone to alert other ants
    // Transitions to: ATTACK (swarm formed), WANDER (danger passed)
    void Update(AntContext& ctx);
  }
}
