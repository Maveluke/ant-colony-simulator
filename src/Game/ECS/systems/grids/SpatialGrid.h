#pragma once
#include "Vec2.h"
#include "ECS/EntityManager.h"
#include "ECS/Constants.h"
#include <vector>
#include <functional>

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

  // Get all entities within radius of position (ALLOCATES - prefer QueryEach)
  std::vector<Entity> Query(const Vec2& position, float radius) const;

  // Zero-allocation query - calls callback for each entity in radius
  // Callback signature: void(Entity e)
  template<typename Func>
  void QueryEach(const Vec2& position, float radius, Func&& callback) const;

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

// =============================================================================
// Template Implementation (must be in header)
// =============================================================================

template<typename Func>
void SpatialGrid::QueryEach(const Vec2& position, float radius, Func&& callback) const {
  // Calculate which cells to check (bounding box of the query circle)
  int minCellX = GetCellX(position.x - radius);
  int maxCellX = GetCellX(position.x + radius);
  int minCellY = GetCellY(position.y - radius);
  int maxCellY = GetCellY(position.y + radius);

  // Check all cells that could contain entities within radius
  for (int cy = minCellY; cy <= maxCellY; cy++) {
    for (int cx = minCellX; cx <= maxCellX; cx++) {
      int index = cy * m_cols + cx;

      // Call callback for each entity in this cell
      for (Entity e : m_cells[index]) {
        callback(e);
      }
    }
  }
}