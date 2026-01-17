#pragma once
#include "ECS/EntityManager.h"
#include "Vec2.h"

namespace SpawnUtils {

  // Spawn an ant at position, belonging to a colony
  // Returns the entity ID of the new ant
  Entity SpawnAnt(EntityManager& em, const Vec2& position);

  // Spawn a food source at position with given amount
  // Returns the entity ID of the new food
  Entity SpawnFood(EntityManager& em, const Vec2& position, float amount = 100.0f);

  // Spawn the colony (ant home base) at position
  // Returns the entity ID of the colony
  Entity SpawnColony(EntityManager& em, const Vec2& position);

  // Spawn a spider (enemy) at position
  // Returns the entity ID of the spider
  Entity SpawnSpider(EntityManager& em, const Vec2& position);

}