#pragma once
#include <map>
#include <array>

#include "ECS/EntityManager.h"
#include "graphics/RenderSystem.h"
#include "Action.h"
#include "Input.h"

namespace MaveLib {
  /*
    Base Scene
  */
  class Scene {
  protected:
    EntityManager entityManager;
    MaveLib::RenderSystem renderSystem;
    std::map<MaveLib::Button, std::string> buttonActionMap;
    std::map<MaveLib::MouseButton, std::string> mouseButtonActionMap;

    int currentFrame = 0;
    bool isPaused = false;
    bool hasEnded = false; // Has the scene ended?

  public:
    Scene() = default;
    virtual ~Scene() = default;

    // Register an action for a specific button
    void registerAction(MaveLib::Button button, const std::string& actionName) {
      buttonActionMap[button] = actionName;
    }

    // Register an action for a specific mouse button
    void registerAction(MaveLib::MouseButton button, const std::string& actionName) {
      mouseButtonActionMap[button] = actionName;
    }

    // Load a new level
    virtual void SLoadLevel() = 0;

    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;

    // Handle user input
    virtual void DoAction(const Action& action) {};
    virtual void DoAction(const MouseAction& action) {};
    virtual void OnAnalogInput(const Vec2& leftStick, const Vec2& rightStick, float leftTrigger, float rightTrigger) {};
    void SProcessInput(
      const std::array<MaveLib::ButtonState, MaveLib::Button::BTN_COUNT>& userButtonStates,
      const std::array<MaveLib::MouseButtonState, MaveLib::MouseButton::MOUSE_BUTTON_COUNT>& userMouseButtonStates,
      const Vec2& leftStick = Vec2{},
      const Vec2& rightStick = Vec2{},
      float leftTrigger = 0.0f,
      float rightTrigger = 0.0f);
  };
}
