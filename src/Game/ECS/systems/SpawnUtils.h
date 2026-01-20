#pragma once
#include "../EntityManager.h"
#include "../Components.h"
#include "../../Vec2.h"

class SpatialGrid;  // Forward declaration

namespace SpawnUtils {

  // Spawn an ant at position belonging to a team
  // Returns the entity ID of the new ant
  Entity SpawnAnt(EntityManager& em, const Vec2& position,
    TeamId team = TEAM_NONE, Entity homeColony = INVALID_ENTITY);

  // Spawn a food source at position with given amount
  // Returns the entity ID of the new food
  Entity SpawnFood(EntityManager& em, const Vec2& position, float amount = 100.0f);

  // Try to spawn food, respecting MAX_FOOD_ENTITIES limit
  // Returns INVALID_ENTITY if limit reached
  Entity TrySpawnFood(EntityManager& em, const Vec2& position, float amount = 100.0f);

  // Try to merge with nearby food, or spawn new if none nearby
  // Returns the food entity (new or merged)
  Entity MergeOrSpawnFood(EntityManager& em, SpatialGrid& grid, const Vec2& position,
    float amount = 100.0f, float mergeRadius = 30.0f);

  // Get current food entity count
  int GetFoodCount(EntityManager& em);

  // Update food visual size, weight, and maxDraggers based on current amount
  // Call this after modifying food amount (e.g., when merging piles)
  void UpdateFoodProperties(EntityManager& em, Entity food);

  // Spawn the colony (ant home base) at position with team color
  // Returns the entity ID of the colony
  Entity SpawnColony(EntityManager& em, const Vec2& position, TeamId team = TEAM_NONE);

  // Spawn a spider (enemy) at position
  // Returns the entity ID of the spider
  Entity SpawnSpider(EntityManager& em, const Vec2& position);

  // Helper to get team colors
  void GetTeamColor(TeamId team, float& r, float& g, float& b);

}