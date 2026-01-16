#pragma once
#include <math.h>
#include <array>

#include "../ContestAPI/app.h"
#include "Scene.h"
#include "ScenePlay.h"

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

class GameEngine {
private:
  GameEngine();
  ~GameEngine();

  std::map<std::string, std::unique_ptr<MaveLib::Scene>> scenes;
  std::string currentSceneSTR;
  bool running;

  std::array<ButtonState, BTN_COUNT> userButtonStates;
  Vec2 leftStickState = Vec2{};
  Vec2 rightStickState = Vec2{};
  float leftTriggerState = 0.0f;
  float rightTriggerState = 0.0f;


  MaveLib::Scene& CurrentScene();
  void ChangeScene(const std::string& nextSceneSTR);

public:
  static GameEngine& Instance();

  void SUserInputHandler();
  void GameSceneInitialize();
  void Init();
  void Update(const float deltaTime);
  void Render();
  void Shutdown();
};