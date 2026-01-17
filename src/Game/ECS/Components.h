#pragma once
#include "Vec2.h"
#include "Definition.h"
// Components class

enum ComponentType {
  NONE = 0,
  TRANSFORM = 1 << 0,
  AABB = 1 << 1,
  HEALTH = 1 << 2,
  COMBAT = 1 << 3,
  ANT = 1 << 4,
  FOOD = 1 << 5,
  COLONY = 1 << 6,
  SPIDER = 1 << 7,
  QUAD_RENDERER = 1 << 8,
  CIRCLE_RENDERER = 1 << 9,
};


struct CTransform {
  Vec2 position = Vec2(0.0, 0.0);
  Vec2 velocity = Vec2(0.0, 0.0);
  Vec2 scale = Vec2(1.0, 1.0);
};


struct CAABB {
  Vec2 size = Vec2(0.0, 0.0);
  Vec2 halfSize = Vec2(0.0, 0.0);
};

struct CHealth {
  float current = 100.0f;
  float max = 100.0f;
};

struct CCombat {
  float attackDamage = 1.0f;      // Damage per attack
  float attackRange = 10.0f;      // Distance required to hit
  float attackCooldown = 0.5f;    // Seconds between attacks
  float attackTimer = 0.0f;       // Current cooldown remaining
  Entity target = 0;              // Entity we're attacking
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
  Vec2 direction = Vec2(0.0f, 0.0f);
  float wanderTimer = 0.0f;                 // Timer for wandering behavior before changing direction
  float speed = 50.0f;
  bool carryingFood = false;
  Entity targetFood = 0;
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

struct CSpider {
  float huntRadius = 150.0f;
  float speed = 80.0f;
};


// Renderer component

// 2D Quad Renderer
struct CQuadRenderer {
  Vec2 size = Vec2(1.0f, 1.0f);
  float z_depth = 0.0f;
  // Color as RGB values between 0 and 1 (default is white)
  float r = 1.0f, g = 1.0f, b = 1.0f;
};

// 2D Circle Renderer
struct CCircleRenderer {
  float radius = 1.0f;
  float z_depth = 0.0f;
  // Color as RGB values between 0 and 1 (default is white)
  float r = 1.0f, g = 1.0f, b = 1.0f;
};