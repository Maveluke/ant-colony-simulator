#pragma once
#include "../Vec2.h"
#include "Constants.h"
#include <array>

constexpr int MAX_DRAGGERS = 8;
constexpr int MAX_FOOD_ENTITIES = 200;
constexpr float MAX_FOOD_AMOUNT = 500.0f;

// TODO: Rearrange later after finalized
enum ComponentType {
  NONE = 0,
  TRANSFORM = 1 << 0,
  AABB = 1 << 1,
  QUAD_RENDERER = 1 << 2,
  CIRCLE_RENDERER = 1 << 3,
  ANT = 1 << 4,
  FOOD = 1 << 5,
  COLONY = 1 << 6,
  SPIDER = 1 << 7,
  HEALTH = 1 << 8,
  COMBAT = 1 << 9,
  CIRCLE_COLLIDER = 1 << 10,
  WANDER = 1 << 11,
  SPEED = 1 << 12,
  DETECTION = 1 << 13,
  TARGET = 1 << 14,
  DRAGGING = 1 << 15,
  DRAGGABLE = 1 << 16
};

struct CTransform {
  Vec2 position = Vec2(0.0f, 0.0f);
  Vec2 velocity = Vec2(0.0f, 0.0f);
  Vec2 scale = Vec2(1.0f, 1.0f);
};

struct CAABB {
  Vec2 size = Vec2(0.0f, 0.0f);
  Vec2 halfSize = Vec2(0.0f, 0.0f);
};

struct CQuadRenderer {
  Vec2 size = Vec2(1.0f, 1.0f);
  float z_depth = 0.0f;
  float r = 1.0f, g = 1.0f, b = 1.0f;
};

struct CCircleRenderer {
  float radius = 1.0f;
  float z_depth = 0.0f;
  float r = 1.0f, g = 1.0f, b = 1.0f;
};

struct CCircleCollider {
  float radius = 1.0f;
};

struct CWander {
  Vec2 direction = Vec2(1.0f, 0.0f);
  float timer = 0.0f;
};

struct CSpeed {
  float value = 50.0f;
};

struct CDetection {
  float radius = 100.0f;
};

// Reference for the target entity
struct CTarget {
  Entity entity = INVALID_ENTITY;
};

// What this entity is dragging (for ants)
struct CDragging {
  Entity target = INVALID_ENTITY;
};

// Can be dragged by other entities (for food, corpses)
struct CDraggable {
  float weight = 1.0f;
  int maxDraggers = 1;
  int draggerCount = 0;
  std::array<Entity, MAX_DRAGGERS> draggers = {};
};

struct CHealth {
  float current = 100.0f;
  float max = 100.0f;
};

struct CCombat {
  float attackDamage = 1.0f;
  float attackCooldown = 0.5f;
  float attackTimer = 0.0f;
};

enum class AntState {
  WANDER,
  FOLLOW_TRAIL,
  FORAGE,
  FLEE,
  ATTACK
};

struct CAnt {
  AntState state = AntState::WANDER;
};

struct CFood {
  float amount = 100.0f;
};

struct CColony {
  float storedFood = 0.0f;
  float spawnThreshold = 10.0f;
  float spawnTimer = 0.0f;
  float spawnCooldown = 0.5f;
};

struct CSpider {};