#pragma once
#include <string>

enum ActionType {
  ACTION_START,   // Just pressed this frame
  ACTION_HOLD,    // Held down
  ACTION_END      // Just released this frame
};

struct Action {
  std::string name;
  ActionType type;
};