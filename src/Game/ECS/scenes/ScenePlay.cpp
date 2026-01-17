#include "ECS/scenes/ScenePlay.h"
#include "../systems/AntSystem.h"
#include "../systems/SpawnUtils.h"
#include "../Constants.h"

namespace MaveLib {
  ScenePlay::ScenePlay()
    : spatialGrid(WORLD_WIDTH, WORLD_HEIGHT) {
  }

  void ScenePlay::SLoadLevel() {
    // Register Action
    registerAction(MaveLib::MouseButton::BUTTON_LEFT, "Spawn");

    // Spawn colony
    Vec2 colonyPos(960.0f, 540.0f);
    SpawnUtils::SpawnColony(entityManager, colonyPos);

    // Spawn ants
    for (int i = 0; i < 500; ++i) {
      SpawnUtils::SpawnAnt(entityManager, colonyPos);
    }
  }

  void ScenePlay::Update(float deltaTime) {
    AntSystem::Update(entityManager, spatialGrid, deltaTime, WORLD_WIDTH, WORLD_HEIGHT);
  }

  void ScenePlay::Render() {
    renderSystem.Render(entityManager);
    DrawUtils::DrawDebugGrid(50.0f, 0.5f, 0.5f, 0.5f);
  }

  void ScenePlay::DoAction(const MouseAction& action) {
    if (action.name == "Spawn" && action.type == ActionType::ACTION_START) {
      printf("Spawning food at (%.1f, %.1f)\n", action.position.x, action.position.y);
      SpawnUtils::SpawnFood(entityManager, action.position, 200.0f);
    }
  }
}