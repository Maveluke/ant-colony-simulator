#include "GameEngine.h"

GameEngine::GameEngine() {
  running = true;
  userButtonStates.fill(MaveLib::ButtonState{});
}

GameEngine::~GameEngine() {
}

void GameEngine::GameSceneInitialize() {
  scenes["Play"] = std::make_unique<MaveLib::ScenePlay>();
  currentSceneSTR = "Play";
  CurrentScene().SLoadLevel();
}

void GameEngine::Init() {
  GameEngine::GameSceneInitialize();
}

MaveLib::Scene& GameEngine::CurrentScene() {
  return *scenes[currentSceneSTR];
}

void GameEngine::ChangeScene(const std::string& nextSceneSTR) {
  currentSceneSTR = nextSceneSTR;
  CurrentScene().SLoadLevel();
}

void GameEngine::SUserInputHandler() {
  constexpr int appButtons[MaveLib::Button::BTN_COUNT] = {
        App::BTN_A,
        App::BTN_B,
        App::BTN_X,
        App::BTN_Y,
        App::BTN_START,
        App::BTN_BACK,
        App::BTN_LBUMPER,
        App::BTN_LSTICK,
        App::BTN_RBUMPER,
        App::BTN_RSTICK,
        App::BTN_DPAD_LEFT,
        App::BTN_DPAD_RIGHT,
        App::BTN_DPAD_UP,
        App::BTN_DPAD_DOWN
  };
  const auto& controller = App::GetController();

  for (size_t i = 0; i < MaveLib::Button::BTN_COUNT; ++i) {
    bool isHeld = controller.CheckButton(static_cast<App::GamepadButton>(appButtons[i]), false);
    MaveLib::ButtonState& btnState = userButtonStates[i];

    btnState.pressed = isHeld && !btnState.held;
    btnState.released = !isHeld && btnState.held;
    btnState.held = isHeld;
  }

  leftStickState = Vec2(controller.GetLeftThumbStickX(), controller.GetLeftThumbStickY());
  rightStickState = Vec2(controller.GetRightThumbStickX(), controller.GetRightThumbStickY());
  leftTriggerState = controller.GetLeftTrigger();
  rightTriggerState = controller.GetRightTrigger();
}

void GameEngine::Update(const float deltaTime) {
  SUserInputHandler();
  CurrentScene().SProcessInput(userButtonStates, leftStickState, rightStickState, leftTriggerState, rightTriggerState);
  CurrentScene().Update(deltaTime);
}

void GameEngine::Render() {
  CurrentScene().Render();
}

void GameEngine::Shutdown() {
  running = false;
}
