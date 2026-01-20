#pragma once
#include "ECS/scenes/Scene.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "../systems/grids/PheromoneGrid.h"
#include "../systems/grids/ColonyPheromoneManager.h"
#include "../systems/EventBuffer.h"
#include "../../graphics/RenderSystem.h"
#include "../../graphics/Camera.h"
#include "DrawUtils.h"

namespace MaveLib {

  enum class SpawnMode {
    FOOD,
    SPIDER,
    PHEROMONE  // Player draws pheromone trails
  };

  /*
    Main gameplay scene with camera, two colonies (blue and red), and ant simulation.
  */
  class ScenePlay : public MaveLib::Scene {
  private:
    SpatialGrid spatialGrid;
    PheromoneGrid pheromoneGrid;           // Shared pheromones (FOOD, ALARM, PLAYER)
    ColonyPheromoneManager colonyPheromones; // Per-team HOME pheromones
    EventBuffer eventBuffer;
    RenderSystem renderSystem;
    Camera camera;

    // Colony entities
    Entity blueColony = INVALID_ENTITY;
    Entity redColony = INVALID_ENTITY;

    // Spawn mode
    SpawnMode currentSpawnMode = SpawnMode::FOOD;

    // Pheromone visualization
    bool showPheromonesToggle = false;
    bool showPheromoneHome = false;
    bool showPheromoneFood = false;
    bool showPheromoneAlarm = false;
    bool showPheromonePlayer = false;
    bool showBlueHome = true;   // Show blue colony's home pheromones
    bool showRedHome = true;    // Show red colony's home pheromones

    // Camera control settings
    static constexpr float CAMERA_MOVE_SPEED = 500.0f;  // World units per second at max stick
    static constexpr float CAMERA_ZOOM_SPEED = 1.0f;    // Zoom change per second at max stick

    // Game constants
    static constexpr float SPAWN_FOOD_AMOUNT = 25.0f;
    static constexpr float SPAWN_FOOD_TIMER = 0.05f;     // in seconds
    static constexpr float MERGE_RADIUS = 15.0f;
    static constexpr float VISIBILITY_THRESHOLD = 5.0f;
    static constexpr float WORLD_BORDER_THICKNESS = 10.0f;
    static constexpr float PLAYER_PHEROMONE_AMOUNT = 50.0f;  // How much pheromone per draw
    static constexpr float PLAYER_PHEROMONE_RADIUS = 10.0f;  // Radius of pheromone deposit

    // Timer
    float spawnFoodTimer = 0.0f;

    void DrawPheromoneLayer(PheromoneType type, float r, float g, float b, bool shouldDraw);
    void DrawColonyHomePheromones(TeamId team, float r, float g, float b, bool shouldDraw);

  public:
    ScenePlay();
    virtual ~ScenePlay() = default;

    void SLoadLevel() override;
    void Update(float deltaTime) override;
    void Render() override;
    void DoAction(const Action& action) override;
    void DoAction(const MouseAction& action) override;
    void OnAnalogInput(const Vec2& leftStick, const Vec2& rightStick,
      float leftTrigger, float rightTrigger) override;
  };
}