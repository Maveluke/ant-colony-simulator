#pragma once
#include "PheromoneGrid.h"
#include "ECS/Components.h"
#include <unordered_map>

/**
 * Manages separate HOME pheromone grids for each colony.
 * Each team (blue, red, etc.) has its own home pheromone layer so ants
 * can navigate back to their own colony without being confused by
 * enemy colony pheromones.
 *
 * Shared pheromone types (FOOD, ALARM, PLAYER) remain on the shared grid.
 */
class ColonyPheromoneManager {
private:
  float m_worldWidth;
  float m_worldHeight;
  float m_cellSize;

  // One home pheromone grid per team
  std::unordered_map<TeamId, PheromoneGrid> m_homeGrids;

public:
  ColonyPheromoneManager(float worldWidth, float worldHeight, float cellSize = 8.0f);

  // Initialize/get home grid for a team (creates if doesn't exist)
  PheromoneGrid& GetHomeGrid(TeamId team);
  const PheromoneGrid& GetHomeGrid(TeamId team) const;

  // Deposit HOME pheromone for a specific team
  void DepositHome(TeamId team, const Vec2& position, float amount);

  // Get HOME pheromone intensity for a specific team
  float GetHomeIntensity(TeamId team, const Vec2& position) const;

  // Update all home grids (decay)
  void Update(float deltaTime);

  // Clear all home grids
  void ClearAll();

  // Check if team has a home grid
  bool HasHomeGrid(TeamId team) const;

  // Get boundary fade multiplier (reduces pheromone near world edges)
  float GetBoundaryMultiplier(const Vec2& position) const;

  // Getters for grid info (for visualization)
  int GetCols() const;
  int GetRows() const;
  float GetCellSize() const { return m_cellSize; }
  float GetCellIntensity(TeamId team, int cellX, int cellY) const;
  float GetMaxCellIntensity(TeamId team) const;
};
