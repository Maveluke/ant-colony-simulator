#pragma once
#include "ECS/scenes/Scene.h"
#include "../systems/SpatialGrid.h"
#include "DrawUtils.h"

namespace MaveLib {
  /*
    Each player is spawned at corresponding spawn point.
    The spawn points should be xOffset from the left(P1)/right(P2) of the screen
    The ball initially spawn at xOffset from the starting player's bar.
  */
  class ScenePlay : public MaveLib::Scene {
  public:
    SpatialGrid spatialGrid;

    ScenePlay();
    virtual ~ScenePlay() = default;

    void SLoadLevel() override;
    void Update(float deltaTime) override;
    void Render() override;
    // void DoAction(const Action& action) override;
    void DoAction(const MouseAction& action) override;
    // void OnAnalogInput(const Vec2& leftStick, const Vec2& rightStick, float leftTrigger, float rightTrigger) override;
  };
}
