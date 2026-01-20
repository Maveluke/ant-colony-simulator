#pragma once
#include "ECS/systems/AntContext.h"

namespace AntStates {
  namespace Forage {

    // FORAGE: Carrying food back to colony
    // DragSystem handles actual movement, this state deposits pheromones
    // and overrides direction toward colony when visible
    void Update(AntContext& ctx);

  }
}
