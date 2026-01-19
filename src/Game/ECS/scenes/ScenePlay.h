#pragma once
#include "ECS/scenes/Scene.h"
#include "ECS/systems/grids/SpatialGrid.h"
#include "../systems/grids/PheromoneGrid.h"
#include "../systems/EventBuffer.h"
#include "../../graphics/RenderSystem.h"
#include "DrawUtils.h"

namespace MaveLib {
  /*
    Each player is spawned at corresponding spawn point.
    The spawn points should be xOffset from the left(P1)/right(P2) of the screen
    The ball initially spawn at xOffset from the starting player's bar.
  */
  class ScenePlay : public MaveLib::Scene {
  private:
    SpatialGrid spatialGrid;
    PheromoneGrid pheromoneGrid;
    EventBuffer eventBuffer;
    RenderSystem renderSystem;

    // Pheromone visualization
    bool showPheromonesToggle = false;
    bool showPheromoneHome = false;
    bool showPheromoneFood = false;
    bool showPheromoneAlarm = false;
    bool showPheromonePlayer = false;

    // Game constants
    static constexpr float SPAWN_FOOD_AMOUNT = 25.0f;
    static constexpr float SPAWN_FOOD_TIMER = 0.05f;     // in seconds
    static constexpr float MERGE_RADIUS = 15.0f;
    static constexpr float VISIBILITY_THRESHOLD = 5.0f;

    // Timer
    float spawnFoodTimer = 0.0f;
    void DrawPheromoneLayer(PheromoneType type, float r, float g, float b, bool shouldDraw);
  public:
    ScenePlay();
    virtual ~ScenePlay() = default;

    void SLoadLevel() override;
    void Update(float deltaTime) override;
    void Render() override;
    void DoAction(const Action& action) override;
    void DoAction(const MouseAction& action) override;
    // void OnAnalogInput(const Vec2& leftStick, const Vec2& rightStick, float leftTrigger, float rightTrigger) override;
  };
}
