#include "PheromoneGrid.h"
#include <cmath>
#include <algorithm>

PheromoneGrid::PheromoneGrid(float worldWidth, float worldHeight, float cellSize)
  : m_worldWidth(worldWidth)
  , m_worldHeight(worldHeight)
  , m_cellSize(cellSize) {

  m_cols = static_cast<int>(std::ceil(worldWidth / cellSize));
  m_rows = static_cast<int>(std::ceil(worldHeight / cellSize));

  int totalCells = m_cols * m_rows;

  // Initialize all layers
  for (int i = 0; i < PHEROMONE_COUNT; i++) {
    m_layers[i].resize(totalCells, 0.0f);
    m_decayTimers[i] = 0.0f;
  }

  // Default configs - stagger the intervals so they don't all update same frame
  // HOME: slow decay, follow descending (back to nest)
  m_configs[PHEROMONE_HOME] = { 0.95f, 0.15f, 255.0f, false };
  m_decayTimers[PHEROMONE_HOME] = 0.0f;

  // FOOD: slow decay, follow descending (toward source)
  m_configs[PHEROMONE_FOOD] = { 0.95f, 0.15f, 255.0f, false };
  m_decayTimers[PHEROMONE_FOOD] = 0.05f;  // Offset

  // ALARM: fast decay, follow ascending (toward danger for attack)
  m_configs[PHEROMONE_ALARM] = { 0.80f, 0.10f, 255.0f, true };
  m_decayTimers[PHEROMONE_ALARM] = 0.10f;  // Offset

  // PLAYER: medium decay, follow ascending (toward where player drew)
  m_configs[PHEROMONE_PLAYER] = { 0.90f, 0.12f, 255.0f, true };
  m_decayTimers[PHEROMONE_PLAYER] = 0.03f;  // Offset
}

int PheromoneGrid::GetCellX(float x) const {
  int cellX = static_cast<int>(x / m_cellSize);
  return std::clamp(cellX, 0, m_cols - 1);
}

int PheromoneGrid::GetCellY(float y) const {
  int cellY = static_cast<int>(y / m_cellSize);
  return std::clamp(cellY, 0, m_rows - 1);
}

int PheromoneGrid::GetCellIndex(int cellX, int cellY) const {
  return cellY * m_cols + cellX;
}

bool PheromoneGrid::IsValidCell(int cellX, int cellY) const {
  return cellX >= 0 && cellX < m_cols && cellY >= 0 && cellY < m_rows;
}

void PheromoneGrid::Deposit(PheromoneType type, const Vec2& position, float amount) {
  int cellX = GetCellX(position.x);
  int cellY = GetCellY(position.y);
  int index = GetCellIndex(cellX, cellY);

  m_layers[type][index] += amount;

  // Cap at max intensity
  if (m_layers[type][index] > m_configs[type].maxIntensity) {
    m_layers[type][index] = m_configs[type].maxIntensity;
  }
}

void PheromoneGrid::DepositRadius(PheromoneType type, const Vec2& center,
  float radius, float amount) {
  // Calculate cell bounds
  int minCellX = GetCellX(center.x - radius);
  int maxCellX = GetCellX(center.x + radius);
  int minCellY = GetCellY(center.y - radius);
  int maxCellY = GetCellY(center.y + radius);

  float radiusSq = radius * radius;
  float maxIntensity = m_configs[type].maxIntensity;

  // Deposit in all cells within radius
  for (int cy = minCellY; cy <= maxCellY; cy++) {
    for (int cx = minCellX; cx <= maxCellX; cx++) {
      // Get cell center position
      float cellCenterX = (cx + 0.5f) * m_cellSize;
      float cellCenterY = (cy + 0.5f) * m_cellSize;

      // Check if cell center is within radius
      float dx = cellCenterX - center.x;
      float dy = cellCenterY - center.y;
      float distSq = dx * dx + dy * dy;

      if (distSq <= radiusSq) {
        int index = GetCellIndex(cx, cy);
        m_layers[type][index] += amount;

        // Cap at max intensity
        if (m_layers[type][index] > maxIntensity) {
          m_layers[type][index] = maxIntensity;
        }
      }
    }
  }
}

void PheromoneGrid::Update(float deltaTime) {
  // Update each pheromone type's decay timer
  for (int type = 0; type < PHEROMONE_COUNT; type++) {
    m_decayTimers[type] -= deltaTime;

    // Time to decay this layer?
    if (m_decayTimers[type] <= 0.0f) {
      // Reset timer
      m_decayTimers[type] = m_configs[type].decayInterval;

      // Apply decay to all cells
      float multiplier = m_configs[type].decayMultiplier;
      for (float& cell : m_layers[type]) {
        cell *= multiplier;

        // Zero out very small values to avoid float dust
        if (cell < 0.01f) {
          cell = 0.0f;
        }
      }
    }
  }
}

Vec2 PheromoneGrid::SampleGradient(PheromoneType type, const Vec2& position) const {
  int centerX = GetCellX(position.x);
  int centerY = GetCellY(position.y);

  // 8 neighbor directions
  // Order: N, NE, E, SE, S, SW, W, NW
  const int dx[] = { 0,  1, 1, 1, 0, -1, -1, -1 };
  const int dy[] = { 1,  1, 0, -1, -1, -1, 0, 1 };

  Vec2 gradient(0.0f, 0.0f);
  float centerIntensity = GetCellIntensity(type, centerX, centerY);
  bool ascending = m_configs[type].followAscending;

  for (int i = 0; i < 8; i++) {
    int nx = centerX + dx[i];
    int ny = centerY + dy[i];

    if (!IsValidCell(nx, ny)) {
      continue;
    }

    float neighborIntensity = GetCellIntensity(type, nx, ny);
    float diff = neighborIntensity - centerIntensity;

    // If ascending, we want to move toward higher values (positive diff)
    // If descending, we want to move toward lower values (negative diff)
    if (!ascending) {
      diff = -diff;
    }

    // Weight the direction by how much stronger/weaker it is
    if (diff > 0.0f) {
      Vec2 dir(static_cast<float>(dx[i]), static_cast<float>(dy[i]));
      gradient += dir.Normalized() * diff;
    }
  }

  // Normalize result if non-zero
  if (gradient.LengthSquared() > 0.0001f) {
    gradient.Normalize();
  }

  return gradient;
}

float PheromoneGrid::GetIntensity(PheromoneType type, const Vec2& position) const {
  int cellX = GetCellX(position.x);
  int cellY = GetCellY(position.y);
  return GetCellIntensity(type, cellX, cellY);
}

float PheromoneGrid::GetCellIntensity(PheromoneType type, int cellX, int cellY) const {
  if (!IsValidCell(cellX, cellY)) {
    return 0.0f;
  }
  return m_layers[type][GetCellIndex(cellX, cellY)];
}

void PheromoneGrid::Clear(PheromoneType type) {
  std::fill(m_layers[type].begin(), m_layers[type].end(), 0.0f);
}

void PheromoneGrid::ClearAll() {
  for (int i = 0; i < PHEROMONE_COUNT; i++) {
    Clear(static_cast<PheromoneType>(i));
  }
}