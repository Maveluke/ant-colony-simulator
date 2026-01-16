#pragma once
#include "ECS/EntityManager.h"

namespace MaveLib {
  /*
    Base Scene
  */
  class Scene {
  private:
    EntityManager entityManager;
    int currentFrame;
    std::vector<std::string> actionMap; // Maps user inputs (refer to GameEngine user input handler) to actions in the scene
    bool isPaused;
    bool hasEnded;

  public:
    Scene() = default;
    virtual ~Scene() = default;
  };

}
