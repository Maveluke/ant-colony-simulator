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
    std::map<MaveLib::Button, std::string> actionMap; // Maps user inputs (refer to GameEngine user input handler) to actions in the scene

    int currentFrame = 0;
    bool isPaused = false;
    bool hasEnded = false;

  public:
    Scene() = default;
    virtual ~Scene() = default;

    void registerAction(MaveLib::Button button, const std::string& actionName) {
      actionMap[button] = actionName;
    }

    virtual void SLoadLevel() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void DoAction(const Action& action) {};
    virtual void OnAnalogInput(const Vec2& leftStick, const Vec2& rightStick, float leftTrigger, float rightTrigger) {}
    void SProcessInput(const std::array<MaveLib::ButtonState, MaveLib::Button::BTN_COUNT>& userButtonStates,
      const Vec2& leftStick = Vec2{},
      const Vec2& rightStick = Vec2{},
      float leftTrigger = 0.0f,
      float rightTrigger = 0.0f);
  };
}
