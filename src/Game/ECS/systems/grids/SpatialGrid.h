#pragma once
#include "Vec2.h"
#include "../EntityManager.h"
#include "../Constants.h"
#include <vector>

class SpatialGrid {
private:
  // Grid configuration
  float m_cellSize;
  int m_cols;
  int m_rows;
  float m_worldWidth;
  float m_worldHeight;

  std::vector<std::vector<Entity>> m_cells;

  // Convert world position to cell index
  int GetCellIndex(float x, float y) const;
  int GetCellX(float x) const;
  int GetCellY(float y) const;

public:
  SpatialGrid(float worldWidth, float worldHeight, float cellSize = 32.0f);

  // Call once per frame before inserting
  void Clear();

  // Add entity to grid based on position
  void Insert(Entity entity, const Vec2& position);

  // Get all entities within radius of position
  std::vector<Entity> Query(const Vec2& position, float radius) const;

  // Get all entities in a specific cell (useful for debugging)
  const std::vector<Entity>& GetCell(int cellX, int cellY) const;

  // Find nearest entity with specific components within radius
  // Returns INVALID_ENTITY if none found
  Entity QueryNearest(const Vec2& position, float radius,
    uint32_t componentMask, EntityManager& em) const;

  // Getters for grid info
  int GetCols() const { return m_cols; }
  int GetRows() const { return m_rows; }
  float GetCellSize() const { return m_cellSize; }
};