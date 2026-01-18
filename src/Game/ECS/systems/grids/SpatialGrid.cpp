#include "SpatialGrid.h"
#include <cmath>
#include <algorithm>

SpatialGrid::SpatialGrid(float worldWidth, float worldHeight, float cellSize)
  : m_worldWidth(worldWidth)
  , m_worldHeight(worldHeight)
  , m_cellSize(cellSize) {
  m_cols = static_cast<int>(std::ceil(worldWidth / cellSize));
  m_rows = static_cast<int>(std::ceil(worldHeight / cellSize));
  m_cells.resize(m_cols * m_rows);
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

      // Add all entities from this cell
      // (Caller can do precise distance check if needed)
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

Entity SpatialGrid::QueryNearest(const Vec2& position, float radius,
  uint32_t componentMask, EntityManager& em) const {
  // Calculate which cells to check
  int minCellX = GetCellX(position.x - radius);
  int maxCellX = GetCellX(position.x + radius);
  int minCellY = GetCellY(position.y - radius);
  int maxCellY = GetCellY(position.y + radius);

  float radiusSq = radius * radius;
  float nearestDistSq = radiusSq + 1.0f;  // Start beyond radius
  Entity nearestEntity = INVALID_ENTITY;

  // Check all cells in bounding box
  for (int cy = minCellY; cy <= maxCellY; cy++) {
    for (int cx = minCellX; cx <= maxCellX; cx++) {
      int index = cy * m_cols + cx;

      for (Entity e : m_cells[index]) {
        // Check if entity has required components
        if (!em.HasComponents(componentMask, e)) {
          continue;
        }

        // Calculate distance
        const Vec2& entityPos = em.GetComponent<CTransform>(e).position;
        Vec2 diff = entityPos - position;
        float distSq = diff.x * diff.x + diff.y * diff.y;

        // Check if within radius and closer than current nearest
        if (distSq <= radiusSq && distSq < nearestDistSq) {
          nearestDistSq = distSq;
          nearestEntity = e;
        }
      }
    }
  }

  return nearestEntity;
}