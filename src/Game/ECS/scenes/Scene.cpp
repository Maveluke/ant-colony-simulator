#include "Scene.h"

void MaveLib::Scene::SProcessInput(const std::array<MaveLib::ButtonState, MaveLib::Button::BTN_COUNT>& userButtonStates,
  const Vec2& leftStick,
  const Vec2& rightStick,
  float leftTrigger,
  float rightTrigger) {
  for (const auto& [button, actionName] : actionMap) {
    const MaveLib::ButtonState& btnState = userButtonStates[button];
    if (btnState.pressed) {
      DoAction(Action{ actionName, ACTION_START });
    }
    else if (btnState.held) {
      DoAction(Action{ actionName, ACTION_HOLD });
    }
    else if (btnState.released) {
      DoAction(Action{ actionName, ACTION_END });
    }
  }

  OnAnalogInput(
    leftStick,
    rightStick,
    leftTrigger,
    rightTrigger
  );
}