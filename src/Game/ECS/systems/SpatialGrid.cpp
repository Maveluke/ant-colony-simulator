#include "SpatialGrid.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

SpatialGrid::SpatialGrid(float worldWidth, float worldHeight, float cellSize)
  : m_worldWidth(worldWidth)
  , m_worldHeight(worldHeight)
  , m_cellSize(cellSize) {
  printf("Ceil(%f) = %d\n", worldWidth / cellSize, static_cast<int>(std::ceil(worldWidth / cellSize)));
  printf("Ceil(%f) = %d\n", worldHeight / cellSize, static_cast<int>(std::ceil(worldHeight / cellSize)));
  m_cols = static_cast<int>(std::ceil(worldWidth / cellSize));
  m_rows = static_cast<int>(std::ceil(worldHeight / cellSize));
  printf("SpatialGrid created with %d cols and %d rows\n", m_cols, m_rows);
  m_cells.resize(m_cols * m_rows);
  printf("SpatialGrid created with %d cells\n", m_cols * m_rows);
}

int SpatialGrid::GetCellX(float x) const {
  int cellX = static_cast<int>(x / m_cellSize);
  return std::clamp(cellX, 0, m_cols - 1);
}

int SpatialGrid::GetCellY(float y) const {
  int cellY = static_cast<int>(y / m_cellSize);
  return std::clamp(cellY, 0, m_rows - 1);
}

int SpatialGrid::GetCellIndex(float x, float y) const {
  return GetCellY(y) * m_cols + GetCellX(x);
}

void SpatialGrid::Clear() {
  for (auto& cell : m_cells) {
    cell.clear();
  }
}

void SpatialGrid::Insert(Entity entity, const Vec2& position) {
  int index = GetCellIndex(position.x, position.y);
  m_cells[index].push_back(entity);
}

std::vector<Entity> SpatialGrid::Query(const Vec2& position, float radius) const {
  std::vector<Entity> result;

  // Calculate which cells to check (bounding box of the query circle)
  int minCellX = GetCellX(position.x - radius);
  int maxCellX = GetCellX(position.x + radius);
  int minCellY = GetCellY(position.y - radius);
  int maxCellY = GetCellY(position.y + radius);

  float radiusSq = radius * radius;

  // Check all cells that could contain entities within radius
  for (int cy = minCellY; cy <= maxCellY; cy++) {
    for (int cx = minCellX; cx <= maxCellX; cx++) {
      int index = cy * m_cols + cx;

      for (Entity e : m_cells[index]) {
        result.push_back(e);
      }
    }
  }

  return result;
}

const std::vector<Entity>& SpatialGrid::GetCell(int cellX, int cellY) const {
  int index = cellY * m_cols + cellX;
  return m_cells[index];
}