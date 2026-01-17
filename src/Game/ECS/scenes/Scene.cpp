#include "Scene.h"

void MaveLib::Scene::SProcessInput(
  const std::array<MaveLib::ButtonState, MaveLib::Button::BTN_COUNT>& userButtonStates,
  const std::array<MaveLib::MouseButtonState, MaveLib::MouseButton::MOUSE_BUTTON_COUNT>& userMouseButtonStates,
  const Vec2& leftStick,
  const Vec2& rightStick,
  float leftTrigger,
  float rightTrigger) {
  for (const auto& [button, actionName] : buttonActionMap) {
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

  for (const auto& [mouseButton, actionName] : mouseButtonActionMap) {
    const MaveLib::MouseButtonState& mouseState = userMouseButtonStates[mouseButton];
    if (mouseState.pressed) {
      DoAction(MouseAction{ actionName, ACTION_START, mouseState.position });
    }
    else if (mouseState.held) {
      DoAction(MouseAction{ actionName, ACTION_HOLD, mouseState.position });
    }
    else if (mouseState.released) {
      DoAction(MouseAction{ actionName, ACTION_END, mouseState.position });
    }
  }

  OnAnalogInput(
    leftStick,
    rightStick,
    leftTrigger,
    rightTrigger
  );
}