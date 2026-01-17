#include "SpawnUtils.h"
#include "../Components.h"
#include "../Definition.h"
#include <cstdlib>
#include <cmath>

namespace SpawnUtils {

  Entity SpawnAnt(EntityManager& em, const Vec2& position) {
    Entity e = em.AddEntityImmediate("ant");

    // Transform
    em.AddComponent(TRANSFORM, e);
    auto& transform = em.GetComponent<CTransform>(e);
    transform.position = position;
    transform.velocity = Vec2(0.0f, 0.0f);

    // Ant behavior
    em.AddComponent(ANT, e);
    auto& ant = em.GetComponent<CAnt>(e);
    ant.state = AntState::WANDER;
    // Random initial direction
    float angle = (rand() / (float)RAND_MAX) * TWO_PI;
    ant.direction = Vec2(cos(angle), sin(angle));
    ant.wanderTimer = 0.0f;
    ant.speed = 50.0f;
    ant.carryingFood = false;
    ant.targetFood = 0;

    // Health (ants are fragile)
    em.AddComponent(HEALTH, e);
    auto& health = em.GetComponent<CHealth>(e);
    health.current = 1.0f;
    health.max = 1.0f;

    // Combat (weak individually)
    em.AddComponent(COMBAT, e);
    auto& combat = em.GetComponent<CCombat>(e);
    combat.attackDamage = 1.0f;
    combat.attackRange = 10.0f;
    combat.attackCooldown = 0.5f;
    combat.attackTimer = 0.0f;
    combat.target = 0;

    // Renderer (small brown circle)
    em.AddComponent(CIRCLE_RENDERER, e);
    auto& renderer = em.GetComponent<CCircleRenderer>(e);
    renderer.radius = 3.0f;
    renderer.r = 0.6f;
    renderer.g = 0.4f;
    renderer.b = 0.2f;

    return e;
  }

  Entity SpawnFood(EntityManager& em, const Vec2& position, float amount) {
    Entity e = em.AddEntityImmediate("food");

    // Transform
    em.AddComponent(TRANSFORM, e);
    auto& transform = em.GetComponent<CTransform>(e);
    transform.position = position;
    transform.velocity = Vec2(0.0f, 0.0f);

    // Food data
    em.AddComponent(FOOD, e);
    auto& food = em.GetComponent<CFood>(e);
    food.amount = amount;

    // Renderer (green circle, size based on amount)
    em.AddComponent(CIRCLE_RENDERER, e);
    auto& renderer = em.GetComponent<CCircleRenderer>(e);
    renderer.radius = 8.0f + (amount / 100.0f) * 7.0f;  // 8-15 radius based on amount
    renderer.r = 0.2f;
    renderer.g = 0.8f;
    renderer.b = 0.2f;

    return e;
  }

  Entity SpawnColony(EntityManager& em, const Vec2& position) {
    Entity e = em.AddEntityImmediate("colony");

    // Transform
    em.AddComponent(TRANSFORM, e);
    auto& transform = em.GetComponent<CTransform>(e);
    transform.position = position;
    transform.velocity = Vec2(0.0f, 0.0f);

    // Colony data
    em.AddComponent(COLONY, e);
    auto& colony = em.GetComponent<CColony>(e);
    colony.storedFood = 0.0f;
    colony.spawnThreshold = 10.0f;
    colony.spawnTimer = 0.0f;
    colony.spawnCooldown = 0.5f;

    // Renderer (dark brown quad)
    em.AddComponent(QUAD_RENDERER, e);
    auto& renderer = em.GetComponent<CQuadRenderer>(e);
    renderer.size = Vec2(40.0f, 40.0f);
    renderer.r = 0.4f;
    renderer.g = 0.2f;
    renderer.b = 0.1f;

    return e;
  }

  Entity SpawnSpider(EntityManager& em, const Vec2& position) {
    Entity e = em.AddEntityImmediate("spider");

    // Transform
    em.AddComponent(TRANSFORM, e);
    auto& transform = em.GetComponent<CTransform>(e);
    transform.position = position;
    transform.velocity = Vec2(0.0f, 0.0f);

    // Spider behavior
    em.AddComponent(SPIDER, e);
    auto& spider = em.GetComponent<CSpider>(e);
    spider.huntRadius = 150.0f;
    spider.speed = 80.0f;

    // Health (tough but killable by swarm)
    em.AddComponent(HEALTH, e);
    auto& health = em.GetComponent<CHealth>(e);
    health.current = 50.0f;
    health.max = 50.0f;

    // Combat (deadly to ants)
    em.AddComponent(COMBAT, e);
    auto& combat = em.GetComponent<CCombat>(e);
    combat.attackDamage = 100.0f;  // One-shots an ant
    combat.attackRange = 15.0f;
    combat.attackCooldown = 0.3f;
    combat.attackTimer = 0.0f;
    combat.target = 0;

    // Renderer (red circle, larger than ants)
    em.AddComponent(CIRCLE_RENDERER, e);
    auto& renderer = em.GetComponent<CCircleRenderer>(e);
    renderer.radius = 12.0f;
    renderer.r = 0.8f;
    renderer.g = 0.1f;
    renderer.b = 0.1f;

    return e;
  }

}