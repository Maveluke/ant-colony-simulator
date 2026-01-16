#pragma once
#include "ECS/scenes/Scene.h"
#include "DrawUtils.h"

namespace MaveLib {
  /*
    Each player is spawned at corresponding spawn point.
    The spawn points should be xOffset from the left(P1)/right(P2) of the screen
    The ball initially spawn at xOffset from the starting player's bar.
  */
  class ScenePlay : public MaveLib::Scene {
  public:
    int p1Score = 0;
    int p2Score = 0;
    Entity p1Entity = -1;
    Entity p2Entity = -1;
    Entity ballEntity = -1;

    ScenePlay() = default;
    virtual ~ScenePlay() = default;

    void SLoadLevel() override;
    void Update(float deltaTime) override;
    void Render() override;
    void DoAction(const Action& action) override;
    void OnAnalogInput(const Vec2& leftStick, const Vec2& rightStick, float leftTrigger, float rightTrigger) override;
    void SCheckGoal();
    void SMovement();
  };
}
