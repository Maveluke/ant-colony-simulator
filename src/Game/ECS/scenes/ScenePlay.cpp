#include "ScenePlay.h"
#include "ECS/systems/AntSystem.h"
#include "ECS/systems/SpiderSystem.h"
#include "ECS/systems/WanderSystem.h"
#include "ECS/systems/AntCollisionSystem.h"
#include "ECS/systems/physics/DragSystem.h"
#include "ECS/systems/physics/MovementSystem.h"
#include "ECS/systems/physics/CollisionSystem.h"
#include "ECS/systems/SpawnUtils.h"
#include "ECS/systems/grids/PheromoneGrid.h"
#include "ECS/systems/grids/ColonyPheromoneManager.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "ECS/Constants.h"

namespace MaveLib {

  // Screen dimensions - adjust these to match your actual screen/window size
  static constexpr float SCREEN_WIDTH = APP_VIRTUAL_WIDTH;
  static constexpr float SCREEN_HEIGHT = APP_VIRTUAL_HEIGHT;

  ScenePlay::ScenePlay()
    : spatialGrid(WORLD_WIDTH, WORLD_HEIGHT, 32.0f),
    pheromoneGrid(WORLD_WIDTH, WORLD_HEIGHT, 8.0f),
    colonyPheromones(WORLD_WIDTH, WORLD_HEIGHT, 8.0f),
    camera(SCREEN_WIDTH, SCREEN_HEIGHT, WORLD_WIDTH, WORLD_HEIGHT) {
  }

  void ScenePlay::SLoadLevel() {
    // Register Action
    registerAction(MaveLib::MouseButton::BUTTON_LEFT, "SpawnFood");
    registerAction(MaveLib::Button::BTN_RSTICK, "TogglePheromonesVisibility");
    registerAction(MaveLib::Button::BTN_DPAD_DOWN, "Toggle1");
    registerAction(MaveLib::Button::BTN_DPAD_LEFT, "Toggle2");
    registerAction(MaveLib::Button::BTN_DPAD_UP, "Toggle3");
    registerAction(MaveLib::Button::BTN_DPAD_RIGHT, "Toggle4");

    // Spawn blue colony (bottom-left)
    Vec2 blueColonyPos(150.0f, 150.0f);
    blueColony = SpawnUtils::SpawnColony(entityManager, blueColonyPos, TEAM_BLUE);

    // Spawn red colony (top-right)
    Vec2 redColonyPos(WORLD_WIDTH - 150.0f, WORLD_HEIGHT - 150.0f);
    redColony = SpawnUtils::SpawnColony(entityManager, redColonyPos, TEAM_RED);

    // Spawn blue ants
    for (int i = 0; i < 500; ++i) {
      SpawnUtils::SpawnAnt(entityManager, blueColonyPos, TEAM_BLUE, blueColony);
    }

    // Spawn red ants
    for (int i = 0; i < 500; ++i) {
      SpawnUtils::SpawnAnt(entityManager, redColonyPos, TEAM_RED, redColony);
    }

    // Set camera to start centered on the world
    camera.SetPosition(Vec2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f));
    camera.SetZoomLimits(0.25f, 4.0f);
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

    // Spider AI - hunt nearest ant
    SpiderSystem::Update(entityManager, spatialGrid, deltaTime);

    // AI systems override velocity
    AntSystem::Update(entityManager, spatialGrid, pheromoneGrid, colonyPheromones, deltaTime);

    // Physics
    DragSystem::Update(entityManager, pheromoneGrid, colonyPheromones, deltaTime);
    MovementSystem::Update(entityManager, deltaTime, WORLD_WIDTH, WORLD_HEIGHT);

    // Collisions → events
    CollisionSystem::Update(entityManager, spatialGrid, eventBuffer);

    // React to events
    AntCollisionSystem::HandleCollisions(entityManager, eventBuffer);

    // Pheromone decay
    pheromoneGrid.Update(deltaTime);
    colonyPheromones.Update(deltaTime);
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

          // Frustum cull pheromone cells
          if (camera.IsRectVisible(Vec2(centerX, centerY), cellSize, cellSize)) {
            DrawUtils::DrawRectangle(camera, centerX, centerY, 0.0f, cellSize, cellSize, r, g, b);
          }
        }
      }
    }
  }

  void ScenePlay::DrawColonyHomePheromones(TeamId team, float baseR, float baseG, float baseB,
    bool shouldDraw) {
    if (!shouldDraw) return;
    if (!colonyPheromones.HasHomeGrid(team)) return;

    float cellSize = colonyPheromones.GetCellSize();
    int cols = colonyPheromones.GetCols();
    int rows = colonyPheromones.GetRows();

    for (int y = 0; y < rows; ++y) {
      for (int x = 0; x < cols; ++x) {
        float intensity = colonyPheromones.GetCellIntensity(team, x, y);
        float maxIntensity = colonyPheromones.GetMaxCellIntensity(team);

        if (intensity > VISIBILITY_THRESHOLD) {
          float colorIntensity = std::min(intensity / maxIntensity, 1.0f);
          float r = baseR * colorIntensity;
          float g = baseG * colorIntensity;
          float b = baseB * colorIntensity;

          float centerX = x * cellSize + cellSize * 0.5f;
          float centerY = y * cellSize + cellSize * 0.5f;

          if (camera.IsRectVisible(Vec2(centerX, centerY), cellSize, cellSize)) {
            DrawUtils::DrawRectangle(camera, centerX, centerY, 0.0f, cellSize, cellSize, r, g, b);
          }
        }
      }
    }
  }


  void ScenePlay::Render() {
    // Draw world border (the "table")
    DrawUtils::DrawWorldBorder(camera, WORLD_WIDTH, WORLD_HEIGHT,
      WORLD_BORDER_THICKNESS, 0.4f, 0.3f, 0.2f);

    // Draw pheromones (if enabled)
    if (showPheromonesToggle) {
      // Draw per-colony home pheromones (blue = cyan, red = magenta)
      if (showPheromoneHome) {
        DrawColonyHomePheromones(TEAM_BLUE, 0.0f, 0.5f, 1.0f, showBlueHome);   // Cyan-blue
        DrawColonyHomePheromones(TEAM_RED, 1.0f, 0.2f, 0.5f, showRedHome);      // Magenta-red
      }
      DrawPheromoneLayer(PHEROMONE_FOOD, 0.0f, 1.0f, 0.3f, showPheromoneFood);
      DrawPheromoneLayer(PHEROMONE_ALARM, 1.0f, 0.2f, 0.2f, showPheromoneAlarm);
      DrawPheromoneLayer(PHEROMONE_PLAYER, 1.0f, 1.0f, 0.0f, showPheromonePlayer);
    }

    // Draw all entities through camera with frustum culling
    renderSystem.Render(entityManager, camera);

    // UI elements (screen space - not affected by camera)
    const char* modeText = "Mode: Food";
    if (currentSpawnMode == SpawnMode::SPIDER) modeText = "Mode: Spider";
    else if (currentSpawnMode == SpawnMode::PHEROMONE) modeText = "Mode: Pheromone";
    DrawUtils::Print(50, 50, modeText, 1.0f, 1.0f, 1.0f);

    // Display zoom level
    char zoomText[32];
    snprintf(zoomText, sizeof(zoomText), "Zoom: %.2fx", camera.GetZoom());
    DrawUtils::Print(50, 80, zoomText, 1.0f, 1.0f, 1.0f);

    // Display camera position
    char posText[64];
    snprintf(posText, sizeof(posText), "Camera: (%.0f, %.0f)",
      camera.GetPosition().x, camera.GetPosition().y);
    DrawUtils::Print(50, 110, posText, 1.0f, 1.0f, 1.0f);

    // Display pheromone visibility status
    if (showPheromonesToggle) {
      DrawUtils::Print(50, 140, "Pheromone View: ON", 0.8f, 1.0f, 0.8f);
      char pheroText[128];
      snprintf(pheroText, sizeof(pheroText), "  HOME:%s  FOOD:%s  ALARM:%s  PLAYER:%s",
        showPheromoneHome ? "ON" : "OFF",
        showPheromoneFood ? "ON" : "OFF",
        showPheromoneAlarm ? "ON" : "OFF",
        showPheromonePlayer ? "ON" : "OFF");
      DrawUtils::Print(50, 170, pheroText, 0.7f, 0.7f, 0.7f);
    }
    else {
      DrawUtils::Print(50, 140, "Pheromone View: OFF", 0.5f, 0.5f, 0.5f);
    }
  }

  void ScenePlay::DoAction(const Action& action) {
    if (action.type == ActionType::ACTION_START) {
      if (action.name == "TogglePheromonesVisibility") {
        showPheromonesToggle = !showPheromonesToggle;
        printf("Pheromones ALL: %s\n", showPheromonesToggle ? "ON" : "OFF");
      }
      else if (action.name == "Toggle1") {
        if (showPheromonesToggle) {
          showPheromoneHome = !showPheromoneHome;
          printf("HOME pheromones: %s\n", showPheromoneHome ? "ON" : "OFF");
        }
        else {
          currentSpawnMode = SpawnMode::FOOD;
          printf("Spawn mode: FOOD\n");
        }
      }
      else if (action.name == "Toggle2") {
        if (showPheromonesToggle) {
          showPheromoneFood = !showPheromoneFood;
          printf("FOOD pheromones: %s\n", showPheromoneFood ? "ON" : "OFF");
        }
        else {
          currentSpawnMode = SpawnMode::SPIDER;
          printf("Spawn mode: SPIDER\n");
        }
      }
      else if (action.name == "Toggle3") {
        if (showPheromonesToggle) {
          showPheromoneAlarm = !showPheromoneAlarm;
          printf("ALARM pheromones: %s\n", showPheromoneAlarm ? "ON" : "OFF");
        }
        else {
          currentSpawnMode = SpawnMode::PHEROMONE;
          printf("Spawn mode: PHEROMONE\n");
        }
      }
      else if (action.name == "Toggle4") {
        if (showPheromonesToggle) {
          showPheromonePlayer = !showPheromonePlayer;
          printf("PLAYER pheromones: %s\n", showPheromonePlayer ? "ON" : "OFF");
        }
      }
    }
  }

  void ScenePlay::DoAction(const MouseAction& action) {
    if (action.name == "SpawnFood" && action.type == ActionType::ACTION_HOLD) {
      if (spawnFoodTimer <= 0.0f) {
        spawnFoodTimer = SPAWN_FOOD_TIMER;

        // Convert screen position to world position through camera
        Vec2 worldPos = camera.ScreenToWorld(action.position);

        // Only spawn if within world bounds
        if (worldPos.x >= 0 && worldPos.x <= WORLD_WIDTH &&
          worldPos.y >= 0 && worldPos.y <= WORLD_HEIGHT) {

          switch (currentSpawnMode) {
            case SpawnMode::FOOD: {
                Entity food = SpawnUtils::TrySpawnFood(entityManager, worldPos, SPAWN_FOOD_AMOUNT);
                if (food != INVALID_ENTITY) {
                  printf("Food spawned at (%.1f, %.1f)\n", worldPos.x, worldPos.y);
                }
                break;
              }
            case SpawnMode::SPIDER: {
                Entity spider = SpawnUtils::SpawnSpider(entityManager, worldPos);
                printf("Spider spawned at (%.1f, %.1f)\n", worldPos.x, worldPos.y);
                break;
              }
            case SpawnMode::PHEROMONE: {
                // Draw player pheromone trail - ants will follow this!
                pheromoneGrid.DepositRadius(PHEROMONE_FOOD, worldPos,
                  PLAYER_PHEROMONE_RADIUS, PLAYER_PHEROMONE_AMOUNT);
                break;
              }
          }
        }
      }
    }
  }

  void ScenePlay::OnAnalogInput(const Vec2& leftStick, const Vec2& rightStick,
    float leftTrigger, float rightTrigger) {
    // Left stick: Move camera
    // Note: We use a fixed deltaTime approximation here since OnAnalogInput
    // doesn't receive deltaTime. For smoother movement, you might want to
    // store the stick values and apply movement in Update()
    const float dt = 1.0f / 60.0f;  // Assume ~60 FPS for now

    if (leftStick.x != 0.0f || leftStick.y != 0.0f) {
      // Scale movement speed inversely with zoom (move slower when zoomed in)
      float effectiveSpeed = CAMERA_MOVE_SPEED / camera.GetZoom();
      Vec2 movement = leftStick * effectiveSpeed * dt;
      camera.Move(movement);
    }

    // Right stick Y-axis: Zoom (push up to zoom in, down to zoom out)
    if (rightStick.y != 0.0f) {
      float zoomDelta = rightStick.y * CAMERA_ZOOM_SPEED * dt;
      camera.AdjustZoom(zoomDelta);
    }

    // Alternative: Use triggers for zoom
    // Right trigger = zoom in, Left trigger = zoom out
    if (rightTrigger > 0.1f || leftTrigger > 0.1f) {
      float zoomDelta = (rightTrigger - leftTrigger) * CAMERA_ZOOM_SPEED * dt;
      camera.AdjustZoom(zoomDelta);
    }
  }

}