#include "ECS/systems/grids/PheromoneGrid.h"
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
  // HOME: slow decay, follow ascending (toward colony)
  m_configs[PHEROMONE_HOME] = { 0.95f, 1.0f, 255.0f };
  m_decayTimers[PHEROMONE_HOME] = 0.0f;

  // FOOD: slow decay, follow descending (toward source)
  m_configs[PHEROMONE_FOOD] = { 0.95f, 1.0f, 255.0f };
  m_decayTimers[PHEROMONE_FOOD] = 0.05f;  // Offset

  // ALARM: fast decay, follow ascending (toward danger for attack)
  m_configs[PHEROMONE_ALARM] = { 0.80f, 0.1f, 255.0f };
  m_decayTimers[PHEROMONE_ALARM] = 0.10f;  // Offset

  // PLAYER: medium decay, follow ascending (toward where player drew)
  m_configs[PHEROMONE_PLAYER] = { 0.90f, 0.12f, 255.0f };
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

// DEPRECATED: Use Monte Carlo sampling in AntSystem instead
// This gradient-based approach averages a small area and can get trapped in local noise
/*
Vec2 PheromoneGrid::SampleGradient(PheromoneType type, const Vec2& position) const {
  int centerX = GetCellX(position.x);
  int centerY = GetCellY(position.y);

  constexpr int SAMPLE_RADIUS = 1;
  bool ascending = m_configs[type].followAscending;

  float totalIntensity = 0.0f;
  int validCells = 0;

  for (int dy = -SAMPLE_RADIUS; dy <= SAMPLE_RADIUS; dy++) {
    for (int dx = -SAMPLE_RADIUS; dx <= SAMPLE_RADIUS; dx++) {
      int nx = centerX + dx;
      int ny = centerY + dy;
      if (!IsValidCell(nx, ny)) continue;
      float cellIntensity = GetCellIntensity(type, nx, ny);
      if (cellIntensity > 0.0f) {
        totalIntensity += cellIntensity;
        validCells++;
      }
    }
  }

  if (validCells == 0) {
    return Vec2(0.0f, 0.0f);
  }

  float avgIntensity = totalIntensity / static_cast<float>(validCells);
  Vec2 gradient(0.0f, 0.0f);

  for (int dy = -SAMPLE_RADIUS; dy <= SAMPLE_RADIUS; dy++) {
    for (int dx = -SAMPLE_RADIUS; dx <= SAMPLE_RADIUS; dx++) {
      if (dx == 0 && dy == 0) continue;
      int nx = centerX + dx;
      int ny = centerY + dy;
      if (!IsValidCell(nx, ny)) continue;

      float cellIntensity = GetCellIntensity(type, nx, ny);
      if (cellIntensity < 0.001f) continue;

      float diff = cellIntensity - avgIntensity;
      if (!ascending) diff = -diff;

      if (diff > 0.0f) {
        Vec2 dir(static_cast<float>(dx), static_cast<float>(dy));
        float dist = dir.Length();
        if (dist > 0.001f) {
          float distanceWeight = 1.0f / (1.0f + (dist - 1.0f) * 0.2f);
          gradient += (dir / dist) * diff * distanceWeight;
        }
      }
    }
  }

  if (gradient.LengthSquared() > 0.0001f) {
    gradient.Normalize();
  }

  return gradient;
}
*/

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

float PheromoneGrid::GetMaxCellIntensity(PheromoneType type) const {
  if (type < 0 || type >= PHEROMONE_COUNT) {
    return 0.0f;
  }
  return m_configs[type].maxIntensity;
}

void PheromoneGrid::Clear(PheromoneType type) {
  std::fill(m_layers[type].begin(), m_layers[type].end(), 0.0f);
}

void PheromoneGrid::ClearAll() {
  for (int i = 0; i < PHEROMONE_COUNT; i++) {
    Clear(static_cast<PheromoneType>(i));
  }
}