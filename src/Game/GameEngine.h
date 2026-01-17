#pragma once
#include <math.h>
#include <array>

#include "../ContestAPI/app.h"
#include "ECS/scenes/Scene.h"
#include "ECS/scenes/ScenePlay.h"
#include "graphics/RenderSystem.h"
#include "Input.h"

class GameEngine {
private:

  std::map<std::string, std::unique_ptr<MaveLib::Scene>> scenes;
  std::string currentSceneSTR;
  bool running;

  MaveLib::RenderSystem renderSystem;

  std::array<MaveLib::ButtonState, MaveLib::Button::BTN_COUNT> userButtonStates;
  std::array<MaveLib::MouseButtonState, MaveLib::MouseButton::MOUSE_BUTTON_COUNT> userMouseButtonStates;
  Vec2 leftStickState = Vec2{};
  Vec2 rightStickState = Vec2{};
  float leftTriggerState = 0.0f;
  float rightTriggerState = 0.0f;


  MaveLib::Scene& CurrentScene();
  void ChangeScene(const std::string& nextSceneSTR);

public:
  GameEngine();
  ~GameEngine();

  void SUserInputHandler();
  void GameSceneInitialize();
  void Init();
  void Update(const float deltaTime);
  void Render();
  void Shutdown();
};