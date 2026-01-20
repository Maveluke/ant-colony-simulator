#pragma once
#include "Vec2.h"
#include <vector>
#include <array>

enum PheromoneType {
  PHEROMONE_HOME,
  PHEROMONE_FOOD,
  PHEROMONE_ALARM,
  PHEROMONE_PLAYER,
  PHEROMONE_COUNT
};

struct PheromoneConfig {
  float decayMultiplier = 0.9f;   // Applied each decay tick (0.9 = lose 10%)
  float decayInterval = 0.1f;     // Seconds between decay ticks
  float maxIntensity = 255.0f;    // Cap value
  float diffusionRate = 0.1f;     // How much spreads to neighbors each tick (0.1 = 10%)
};

class PheromoneGrid {
private:
  // Grid dimensions
  float m_cellSize;
  int m_cols;
  int m_rows;
  float m_worldWidth;
  float m_worldHeight;

  // One flat array per pheromone type
  std::array<std::vector<float>, PHEROMONE_COUNT> m_layers;

  // Config and timers per type (for staggered decay)
  std::array<PheromoneConfig, PHEROMONE_COUNT> m_configs;
  std::array<float, PHEROMONE_COUNT> m_decayTimers;

  // Temp buffer for diffusion (reused to avoid allocations)
  std::vector<float> m_diffusionBuffer;

  // Helpers
  int GetCellX(float x) const;
  int GetCellY(float y) const;
  int GetCellIndex(int cellX, int cellY) const;
  bool IsValidCell(int cellX, int cellY) const;

public:
  PheromoneGrid(float worldWidth, float worldHeight, float cellSize = 16.0f);

  // Deposit pheromone at world position
  void Deposit(PheromoneType type, const Vec2& position, float amount);

  // Deposit pheromone in a radius around position (spreads amount across cells)
  void DepositRadius(PheromoneType type, const Vec2& center, float radius, float amount);

  // Call once per frame - handles staggered decay internally
  void Update(float deltaTime);

  // DEPRECATED: Use Monte Carlo sampling in AntSystem instead
  // Get direction to follow based on pheromone config (ascending or descending)
  // Samples a 5x5 area and compares to average intensity for more stable gradients
  // Returns zero vector if no gradient detected
  // Vec2 SampleGradient(PheromoneType type, const Vec2& position) const;

  // Get intensity at position (for visualization / threshold checks)
  float GetIntensity(PheromoneType type, const Vec2& position) const;

  // Clear one layer or all
  void Clear(PheromoneType type);
  void ClearAll();

  // Getters for grid info (useful for visualization)
  int GetCols() const { return m_cols; }
  int GetRows() const { return m_rows; }
  float GetCellSize() const { return m_cellSize; }

  // Direct cell access for visualization
  float GetCellIntensity(PheromoneType type, int cellX, int cellY) const;
  float GetMaxCellIntensity(PheromoneType type) const;
};