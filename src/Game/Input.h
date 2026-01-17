#pragma once
namespace MaveLib {
  enum Button {
    BTN_A,
    BTN_B,
    BTN_X,
    BTN_Y,

    BTN_START,
    BTN_BACK,

    BTN_LBUMPER,
    BTN_LSTICK,
    BTN_RBUMPER,
    BTN_RSTICK,

    BTN_DPAD_LEFT,
    BTN_DPAD_RIGHT,
    BTN_DPAD_UP,
    BTN_DPAD_DOWN,
    BTN_COUNT
  };

  struct ButtonState {
    bool pressed = false;
    bool held = false;
    bool released = false;
  };

  enum MouseButton {
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_MIDDLE,
    MOUSE_BUTTON_COUNT
  };

  struct MouseButtonState {
    bool pressed = false;
    bool held = false;
    bool released = false;
    Vec2 position;
  };
}
