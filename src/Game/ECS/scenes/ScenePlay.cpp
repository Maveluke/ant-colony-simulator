#include "ScenePlay.h"
#include "ECS/systems/AntSystem.h"
#include "ECS/systems/WanderSystem.h"
#include "ECS/systems/physics/DragSystem.h"
#include "ECS/systems/physics/MovementSystem.h"
#include "ECS/systems/physics/CollisionSystem.h"
#include "ECS/systems/SpawnUtils.h"
#include "ECS/systems/grids/PheromoneGrid.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "ECS/Constants.h"

namespace MaveLib {

  ScenePlay::ScenePlay()
    : spatialGrid(WORLD_WIDTH, WORLD_HEIGHT, 32.0f),
    pheromoneGrid(WORLD_WIDTH, WORLD_HEIGHT, 8.0f) {
  }

  void ScenePlay::SLoadLevel() {
    // Register Action
    registerAction(MaveLib::MouseButton::BUTTON_LEFT, "SpawnFood");
    registerAction(MaveLib::Button::BTN_RSTICK, "TogglePheromonesVisibility");
    registerAction(MaveLib::Button::BTN_DPAD_DOWN, "Toggle1");
    registerAction(MaveLib::Button::BTN_DPAD_LEFT, "Toggle2");
    registerAction(MaveLib::Button::BTN_DPAD_UP, "Toggle3");
    registerAction(MaveLib::Button::BTN_DPAD_RIGHT, "Toggle4");

    // Spawn colony
    Vec2 colonyPos(100.0f, 100.0f);
    SpawnUtils::SpawnColony(entityManager, colonyPos);

    // Spawn ants
    for (int i = 0; i < 1000; ++i) {
      SpawnUtils::SpawnAnt(entityManager, colonyPos);
    }
  }

  void ScenePlay::Update(float deltaTime) {
    // Count down timers
    if (spawnFoodTimer > 0.0f) {
      spawnFoodTimer -= deltaTime;
    }

    // Clear events
    eventBuffer.ClearAll();

    // Rebuild spatial grid (all entities with TRANSFORM)
    spatialGrid.Clear();
    auto transformEntities = entityManager.GetEntitiesWithComponents(TRANSFORM);
    for (Entity e : transformEntities) {
      Vec2 pos = entityManager.GetComponent<CTransform>(e).position;
      spatialGrid.Insert(e, pos);
    }

    // Base wandering (sets velocity for all wanderers)
    WanderSystem::Update(entityManager, deltaTime);

    // AI systems override velocity
    AntSystem::Update(entityManager, spatialGrid, pheromoneGrid, deltaTime);

    // Physics
    DragSystem::Update(entityManager, pheromoneGrid, deltaTime);
    MovementSystem::Update(entityManager, deltaTime, WORLD_WIDTH, WORLD_HEIGHT);

    // Collisions → events
    CollisionSystem::Update(entityManager, spatialGrid, eventBuffer);

    // React to events
    AntSystem::HandleCollisions(entityManager, eventBuffer);

    // Pheromone decay
    pheromoneGrid.Update(deltaTime);
  }

  void ScenePlay::DrawPheromoneLayer(PheromoneType type, float baseR, float baseG, float baseB,
    bool shouldDraw) {
    if (!shouldDraw) return;

    float cellSize = pheromoneGrid.GetCellSize();
    int cols = pheromoneGrid.GetCols();
    int rows = pheromoneGrid.GetRows();

    for (int y = 0; y < rows; ++y) {
      for (int x = 0; x < cols; ++x) {
        float intensity = pheromoneGrid.GetCellIntensity(type, x, y);
        float maxIntensity = pheromoneGrid.GetMaxCellIntensity(type);

        if (intensity > VISIBILITY_THRESHOLD) {
          // Fake alpha by scaling color intensity (brighter = stronger)
          float colorIntensity = std::min(intensity / maxIntensity, 1.0f);
          float r = baseR * colorIntensity;
          float g = baseG * colorIntensity;
          float b = baseB * colorIntensity;

          // Center rect at cell center, z=0.0 (below entities)
          float centerX = x * cellSize + cellSize * 0.5f;
          float centerY = y * cellSize + cellSize * 0.5f;

          DrawUtils::DrawRectangle(centerX, centerY, 0.0f, cellSize, cellSize, r, g, b);
        }
      }
    }
  }


  void ScenePlay::Render() {
    if (showPheromonesToggle) {
      DrawPheromoneLayer(PHEROMONE_HOME, 0.0f, 0.3f, 1.0f, showPheromoneHome);
      DrawPheromoneLayer(PHEROMONE_FOOD, 0.0f, 1.0f, 0.3f, showPheromoneFood);
      DrawPheromoneLayer(PHEROMONE_ALARM, 1.0f, 0.2f, 0.2f, showPheromoneAlarm);
      DrawPheromoneLayer(PHEROMONE_PLAYER, 1.0f, 1.0f, 0.0f, showPheromonePlayer);
    }

    renderSystem.Render(entityManager);

    DrawUtils::DrawDebugGrid(50.0f, 0.5f, 0.5f, 0.5f);
  }

  void ScenePlay::DoAction(const Action& action) {
    if (action.type == ActionType::ACTION_START) {
      if (action.name == "TogglePheromonesVisibility") {
        showPheromonesToggle = !showPheromonesToggle;
        printf("Pheromones ALL: %s\n", showPheromonesToggle ? "ON" : "OFF");
      }
      else if (action.name == "Toggle1") {
        showPheromoneHome = !showPheromoneHome;
        printf("HOME pheromones: %s\n", showPheromoneHome ? "ON" : "OFF");
      }
      else if (action.name == "Toggle2") {
        showPheromoneFood = !showPheromoneFood;
        printf("FOOD pheromones: %s\n", showPheromoneFood ? "ON" : "OFF");
      }
      else if (action.name == "Toggle3") {
        showPheromoneAlarm = !showPheromoneAlarm;
        printf("ALARM pheromones: %s\n", showPheromoneAlarm ? "ON" : "OFF");
      }
      else if (action.name == "Toggle4") {
        showPheromonePlayer = !showPheromonePlayer;
        printf("PLAYER pheromones: %s\n", showPheromonePlayer ? "ON" : "OFF");
      }
    }

  }

  void ScenePlay::DoAction(const MouseAction& action) {
    if (action.name == "SpawnFood" && action.type == ActionType::ACTION_HOLD) {
      if (spawnFoodTimer <= 0.0f) {
        spawnFoodTimer = SPAWN_FOOD_TIMER;
        Entity food = SpawnUtils::MergeOrSpawnFood(entityManager, spatialGrid,
          action.position,
          SPAWN_FOOD_AMOUNT, MERGE_RADIUS);

        if (food != INVALID_ENTITY) {
          printf("Food pile at (%.1f, %.1f): %s\n",
            action.position.x, action.position.y,
            food == INVALID_ENTITY ? "MERGED" : "SPAWNED");
        }
      }
    }

  }
}
