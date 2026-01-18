#pragma once
#include "Vec2.h"
#include "Constants.h"

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
  CARRYING = 1 << 15
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

struct CTarget {
  Entity entity = INVALID_ENTITY;
};

struct CCarrying {
  float foodAmount = 0.0f;
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