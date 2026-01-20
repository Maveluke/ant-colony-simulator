#include "SpawnUtils.h"
#include "grids/SpatialGrid.h"
#include "../Components.h"
#include "../Constants.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

namespace SpawnUtils {

  // Helper: Random float between min and max
  static float RandomFloat(float min, float max) {
    return min + (rand() / (float)RAND_MAX) * (max - min);
  }

  // Helper: Calculate max draggers based on food amount
  static int CalculateMaxDraggers(float foodAmount) {
    int draggers = static_cast<int>(foodAmount / 50.0f) + 1;
    return std::clamp(draggers, 1, MAX_DRAGGERS);
  }

  // Helper: Calculate weight based on food amount
  static float CalculateWeight(float foodAmount) {
    return 1.0f + (foodAmount / 50.0f);
  }

  // Helper: Calculate visual radius based on food amount
  static float CalculateFoodRadius(float foodAmount) {
    return 8.0f + std::min(foodAmount / 100.0f, 1.0f) * 12.0f;  // 8-20 radius
  }

  void GetTeamColor(TeamId team, float& r, float& g, float& b) {
    switch (team) {
      case TEAM_BLUE:
        r = 0.2f; g = 0.4f; b = 0.9f;  // Blue
        break;
      case TEAM_RED:
        r = 0.9f; g = 0.2f; b = 0.2f;  // Red
        break;
      case TEAM_NONE:
      default:
        r = 0.6f; g = 0.4f; b = 0.2f;  // Brown (original)
        break;
    }
  }

  Entity SpawnAnt(EntityManager& em, const Vec2& position, TeamId team, Entity homeColony) {
    Entity e = em.AddEntityImmediate("ant");

    // Transform
    em.AddComponent(TRANSFORM, e);
    auto& transform = em.GetComponent<CTransform>(e);
    transform.position = position;
    transform.velocity = Vec2(0.0f, 0.0f);

    // Collider (small)
    em.AddComponent(CIRCLE_COLLIDER, e);
    auto& collider = em.GetComponent<CCircleCollider>(e);
    collider.radius = 3.0f;

    // Ant marker with team info
    em.AddComponent(ANT, e);
    auto& ant = em.GetComponent<CAnt>(e);
    ant.state = AntState::WANDER;
    ant.teamId = team;
    ant.homeColony = homeColony;

    // Wander behavior
    em.AddComponent(WANDER, e);
    auto& wander = em.GetComponent<CWander>(e);
    float angle = RandomFloat(0.0f, TWO_PI);
    wander.direction = Vec2(cos(angle), sin(angle));
    wander.timer = 0.0f;

    // Speed
    em.AddComponent(SPEED, e);
    auto& speed = em.GetComponent<CSpeed>(e);
    speed.value = 40.0f;

    // Detection
    em.AddComponent(DETECTION, e);
    auto& detection = em.GetComponent<CDetection>(e);
    detection.radius = 50.0f;

    // Target (initially none)
    em.AddComponent(TARGET, e);

    // Dragging (initially nothing)
    em.AddComponent(DRAGGING, e);
    auto& dragging = em.GetComponent<CDragging>(e);
    dragging.target = INVALID_ENTITY;

    // Health
    em.AddComponent(HEALTH, e);
    auto& health = em.GetComponent<CHealth>(e);
    health.current = 5.0f;
    health.max = 5.0f;

    // Combat
    em.AddComponent(COMBAT, e);
    auto& combat = em.GetComponent<CCombat>(e);
    combat.attackDamage = 1.0f;
    combat.attackCooldown = 0.5f;
    combat.attackTimer = 0.0f;

    // Renderer - color based on team
    em.AddComponent(CIRCLE_RENDERER, e);
    auto& renderer = em.GetComponent<CCircleRenderer>(e);
    renderer.radius = collider.radius;
    GetTeamColor(team, renderer.r, renderer.g, renderer.b);

    return e;
  }

  Entity SpawnFood(EntityManager& em, const Vec2& position, float amount) {
    // Clamp amount to max
    amount = std::min(amount, MAX_FOOD_AMOUNT);

    Entity e = em.AddEntityImmediate("food");

    // Transform
    em.AddComponent(TRANSFORM, e);
    auto& transform = em.GetComponent<CTransform>(e);
    transform.position = position;
    transform.velocity = Vec2(0.0f, 0.0f);

    float radius = CalculateFoodRadius(amount);

    // Collider (size based on amount)
    em.AddComponent(CIRCLE_COLLIDER, e);
    auto& collider = em.GetComponent<CCircleCollider>(e);
    collider.radius = radius;

    // Food data
    em.AddComponent(FOOD, e);
    auto& food = em.GetComponent<CFood>(e);
    food.amount = amount;

    // Draggable
    em.AddComponent(DRAGGABLE, e);
    auto& draggable = em.GetComponent<CDraggable>(e);
    draggable.weight = CalculateWeight(amount);
    draggable.maxDraggers = CalculateMaxDraggers(amount);
    draggable.draggerCount = 0;

    // Renderer (green)
    em.AddComponent(CIRCLE_RENDERER, e);
    auto& renderer = em.GetComponent<CCircleRenderer>(e);
    renderer.radius = radius;
    renderer.r = 0.2f;
    renderer.g = 0.8f;
    renderer.b = 0.2f;

    return e;
  }

  void UpdateFoodProperties(EntityManager& em, Entity food) {
    if (!em.HasComponents(FOOD | DRAGGABLE | CIRCLE_COLLIDER | CIRCLE_RENDERER, food)) return;

    auto& foodComp = em.GetComponent<CFood>(food);
    auto& draggable = em.GetComponent<CDraggable>(food);
    auto& collider = em.GetComponent<CCircleCollider>(food);
    auto& renderer = em.GetComponent<CCircleRenderer>(food);

    // Clamp amount to max
    foodComp.amount = std::min(foodComp.amount, MAX_FOOD_AMOUNT);

    // Update based on current amount
    float radius = CalculateFoodRadius(foodComp.amount);
    draggable.weight = CalculateWeight(foodComp.amount);
    draggable.maxDraggers = CalculateMaxDraggers(foodComp.amount);
    collider.radius = radius;
    renderer.radius = radius;
  }

  int GetFoodCount(EntityManager& em) {
    return static_cast<int>(em.GetEntitiesWithComponents(FOOD).size());
  }

  Entity TrySpawnFood(EntityManager& em, const Vec2& position, float amount) {
    // Check if we can spawn more food
    if (GetFoodCount(em) >= MAX_FOOD_ENTITIES) {
      return INVALID_ENTITY;
    }
    return SpawnFood(em, position, amount);
  }

  Entity MergeOrSpawnFood(EntityManager& em, SpatialGrid& grid, const Vec2& position,
    float amount, float mergeRadius) {
    // Look for nearby food to merge with
    Entity nearbyFood = grid.QueryNearest(position, mergeRadius, FOOD | TRANSFORM, em);

    if (nearbyFood != INVALID_ENTITY) {
      // Merge with existing food
      auto& foodComp = em.GetComponent<CFood>(nearbyFood);
      foodComp.amount += amount;
      UpdateFoodProperties(em, nearbyFood);
      return nearbyFood;
    }

    // No nearby food, try to spawn new
    return TrySpawnFood(em, position, amount);
  }

  Entity SpawnColony(EntityManager& em, const Vec2& position, TeamId team) {
    Entity e = em.AddEntityImmediate("colony");

    // Transform
    em.AddComponent(TRANSFORM, e);
    auto& transform = em.GetComponent<CTransform>(e);
    transform.position = position;
    transform.velocity = Vec2(0.0f, 0.0f);

    // Colony data with team
    em.AddComponent(COLONY, e);
    auto& colony = em.GetComponent<CColony>(e);
    colony.storedFood = 0.0f;
    colony.spawnThreshold = 10.0f;
    colony.spawnTimer = 0.0f;
    colony.spawnCooldown = 0.5f;
    colony.teamId = team;

    // Renderer - color based on team (darker shade)
    em.AddComponent(QUAD_RENDERER, e);
    auto& renderer = em.GetComponent<CQuadRenderer>(e);
    renderer.size = Vec2(40.0f, 40.0f);

    // Get team color but make it darker for the colony
    float r, g, b;
    GetTeamColor(team, r, g, b);
    renderer.r = r * 0.6f;  // Darker version
    renderer.g = g * 0.6f;
    renderer.b = b * 0.6f;

    return e;
  }

  Entity SpawnSpider(EntityManager& em, const Vec2& position) {
    Entity e = em.AddEntityImmediate("spider");

    // Transform
    em.AddComponent(TRANSFORM, e);
    auto& transform = em.GetComponent<CTransform>(e);
    transform.position = position;
    transform.velocity = Vec2(0.0f, 0.0f);

    // Collider
    em.AddComponent(CIRCLE_COLLIDER, e);
    auto& collider = em.GetComponent<CCircleCollider>(e);
    collider.radius = 12.0f;

    // Spider marker
    em.AddComponent(SPIDER, e);

    // Wander behavior (when no target)
    em.AddComponent(WANDER, e);
    auto& wander = em.GetComponent<CWander>(e);
    float angle = RandomFloat(0.0f, TWO_PI);
    wander.direction = Vec2(cos(angle), sin(angle));
    wander.timer = 0.0f;

    // Speed
    em.AddComponent(SPEED, e);
    auto& speed = em.GetComponent<CSpeed>(e);
    speed.value = 50.0f;

    // Detection (hunt radius)
    em.AddComponent(DETECTION, e);
    auto& detection = em.GetComponent<CDetection>(e);
    detection.radius = 100.0f;

    // Target (initially none)
    em.AddComponent(TARGET, e);

    // Health
    em.AddComponent(HEALTH, e);
    auto& health = em.GetComponent<CHealth>(e);
    health.current = 50.0f;
    health.max = 50.0f;

    // Combat
    em.AddComponent(COMBAT, e);
    auto& combat = em.GetComponent<CCombat>(e);
    combat.attackDamage = 100.0f;
    combat.attackCooldown = 1.0f;
    combat.attackTimer = 0.0f;

    // Renderer (dark purple for spider - distinct from teams)
    em.AddComponent(CIRCLE_RENDERER, e);
    auto& renderer = em.GetComponent<CCircleRenderer>(e);
    renderer.radius = collider.radius;
    renderer.r = 0.5f;
    renderer.g = 0.1f;
    renderer.b = 0.5f;

    return e;
  }

}