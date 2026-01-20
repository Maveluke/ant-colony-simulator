#pragma once
#include "ECS/systems/AntContext.h"

namespace AntStates {
  namespace Attack {

    // ATTACK: Moving toward spider to fight
    // Deposits alarm pheromone to maintain swarm
    // Transitions to: FLEE (swarm dispersed), WANDER (danger passed)
    void Update(AntContext& ctx);
  }
}
