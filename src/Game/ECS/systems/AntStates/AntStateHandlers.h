#pragma once
#include "ECS/systems/AntContext.h"
#include "ECS/systems/AntStates/WanderState.h"
#include "ECS/systems/AntStates/FollowTrailState.h"
#include "ECS/systems/AntStates/ForageState.h"
#include "ECS/systems/AntStates/FleeState.h"
#include "ECS/systems/AntStates/AttackState.h"

namespace AntStateHandlers {

  // Dispatch to appropriate state handler based on current ant state
  inline void Update(AntContext& ctx) {
    switch (ctx.ant.state) {
      case AntState::WANDER:
        AntStates::Wander::Update(ctx);
        break;
      case AntState::FOLLOW_TRAIL:
        AntStates::FollowTrail::Update(ctx);
        break;
      case AntState::FORAGE:
        AntStates::Forage::Update(ctx);
        break;
      case AntState::FLEE:
        AntStates::Flee::Update(ctx);
        break;
      case AntState::ATTACK:
        AntStates::Attack::Update(ctx);
        break;
    }
  }
}
