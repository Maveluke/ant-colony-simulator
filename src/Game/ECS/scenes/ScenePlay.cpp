#include "ECS/scenes/ScenePlay.h"
#include "../ContestAPI/app.h"
#include "ErrorLog.h"

namespace MaveLib {
  void ScenePlay::SLoadLevel() {

    // Clear previous entities
    entityManager = EntityManager();
    p1Score = 0;
    p2Score = 0;

    // Add Player 1
    entityManager.AddEntity("Player1");
    entityManager.Update(); // Ensure the entity is created before adding components
    p1Entity = entityManager.GetEntities("Player1")[0];
    if (!entityManager.AddComponent(ComponentType::PLAYER, p1Entity)) {
      ErrorLog("Failed to add PLAYER component to Player1");
    }
    if (!entityManager.AddComponent(ComponentType::TRANSFORM, p1Entity)) {
      ErrorLog("Failed to add TRANSFORM component to Player1");
    }
    if (!entityManager.AddComponent(ComponentType::AABB, p1Entity)) {
      ErrorLog("Failed to add AABB component to Player1");
    }
    if (!entityManager.AddComponent(ComponentType::QUAD_RENDERER, p1Entity)) {
      ErrorLog("Failed to add QUAD_RENDERER component to Player1");
    }

    auto& p1Transform = entityManager.GetComponent<CTransform>(p1Entity);
    auto& p1AABB = entityManager.GetComponent<CAABB>(p1Entity);
    auto& p1Renderer = entityManager.GetComponent<CQuadRenderer>(p1Entity);

    const int xOffset = 100;

    p1Transform.position = Vec2(xOffset, APP_VIRTUAL_HEIGHT / 2.0f);
    p1Transform.scale = Vec2(1.0f, 1.0f);
    p1AABB.size = Vec2(25.0f, 100.0f);
    p1AABB.halfSize = p1AABB.size / 2.0f;
    p1Renderer.size = p1AABB.size;
    p1Renderer.r = 0.0f; p1Renderer.g = 0.0f; p1Renderer.b = 1.0f;

    // Add Player 2
    entityManager.AddEntity("Player2");
    entityManager.Update(); // Ensure the entity is created before adding components
    p2Entity = entityManager.GetEntities("Player2")[0];
    entityManager.AddComponent(ComponentType::PLAYER, p2Entity);
    entityManager.AddComponent(ComponentType::TRANSFORM, p2Entity);
    entityManager.AddComponent(ComponentType::AABB, p2Entity);
    entityManager.AddComponent(ComponentType::QUAD_RENDERER, p2Entity);

    auto& p2Player = entityManager.GetComponent<CPlayer>(p2Entity);
    auto& p2Transform = entityManager.GetComponent<CTransform>(p2Entity);
    auto& p2AABB = entityManager.GetComponent<CAABB>(p2Entity);
    auto& p2Renderer = entityManager.GetComponent<CQuadRenderer>(p2Entity);

    p2Player.isLeftPlayer = false;
    p2Transform.position = Vec2(APP_VIRTUAL_WIDTH - xOffset, APP_VIRTUAL_HEIGHT / 2.0f);
    p2Transform.scale = Vec2(1.0f, 1.0f);
    p2AABB.size = Vec2(25.0f, 100.0f);
    p2AABB.halfSize = p2AABB.size / 2.0f;
    p2Renderer.size = p2AABB.size;
    p2Renderer.r = 1.0f; p2Renderer.g = 0.0f; p2Renderer.b = 0.0f;

    // Add Ball
    entityManager.AddEntity("Ball");
    entityManager.Update(); // Ensure the entity is created before adding components
    ballEntity = entityManager.GetEntities("Ball")[0];
    entityManager.AddComponent(ComponentType::TRANSFORM, ballEntity);
    entityManager.AddComponent(ComponentType::CIRCLE_RENDERER, ballEntity);
    entityManager.AddComponent(ComponentType::AABB, ballEntity);
    auto& ballTransform = entityManager.GetComponent<CTransform>(ballEntity);
    auto& ballRenderer = entityManager.GetComponent<CCircleRenderer>(ballEntity);
    auto& ballAABB = entityManager.GetComponent<CAABB>(ballEntity);

    ballTransform.position = Vec2(APP_VIRTUAL_WIDTH / 2.0f, APP_VIRTUAL_HEIGHT / 2.0f);
    ballTransform.scale = Vec2(1.0f, 1.0f);
    ballAABB.size = Vec2(20.0f, 20.0f);
    ballAABB.halfSize = ballAABB.size / 2.0f;
    ballRenderer.radius = 10.0f;
    ballRenderer.r = 1.0f; ballRenderer.g = 1.0f; ballRenderer.b = 1.0f;
  }

  void ScenePlay::Update(float deltaTime) {
    SMovement();
  }

  void ScenePlay::Render() {
    renderSystem.Render(entityManager);
    DrawUtils::DrawDebugGrid(50.0f, 0.5f, 0.5f, 0.5f);
  }

  void ScenePlay::DoAction(const Action& action) {
  }

  void ScenePlay::OnAnalogInput(const Vec2& leftStick, const Vec2& rightStick, float leftTrigger, float rightTrigger) {
    if (entityManager.HasComponents(ComponentType::PLAYER, p1Entity)) {
      auto& p1Player = entityManager.GetComponent<CPlayer>(p1Entity);
      p1Player.moveInput = leftStick.y;
      if (leftStick.y > 0.0f || leftStick.y < 0.0f) printf("P1 move input: %f\n", p1Player.moveInput);

    }
    if (entityManager.HasComponents(ComponentType::PLAYER, p2Entity)) {
      auto& p2Player = entityManager.GetComponent<CPlayer>(p2Entity);
      p2Player.moveInput = rightStick.y;
      if (rightStick.y > 0.0f || rightStick.y < 0.0f) printf("P2 move input: %f\n", p2Player.moveInput);
    }
  }

  void ScenePlay::SMovement() {
    const float playerSpeed = 400.0f; // Units per second

    uint32_t componentMask = ComponentType::PLAYER | ComponentType::TRANSFORM;
    // Update Player 1 movement
    if (entityManager.HasComponents(componentMask, p1Entity)) {
      auto& p1Player = entityManager.GetComponent<CPlayer>(p1Entity);
      auto& p1Transform = entityManager.GetComponent<CTransform>(p1Entity);
      p1Transform.velocity.y = p1Player.moveInput * playerSpeed;
      p1Transform.position += p1Transform.velocity * (1.0f / 60.0f); // Assuming 60 FPS for simplicity
    }
    // Update Player 2 movement
    if (entityManager.HasComponents(componentMask, p2Entity)) {
      auto& p2Player = entityManager.GetComponent<CPlayer>(p2Entity);
      auto& p2Transform = entityManager.GetComponent<CTransform>(p2Entity);
      p2Transform.velocity.y = p2Player.moveInput * playerSpeed;
      p2Transform.position += p2Transform.velocity * (1.0f / 60.0f); // Assuming 60 FPS for simplicity
    }
  }

}