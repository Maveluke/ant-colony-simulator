#include "ColonyPheromoneManager.h"
#include <stdexcept>
#include <algorithm>

// Distance from boundary where pheromone starts to fade
static constexpr float BOUNDARY_FADE_DISTANCE = 100.0f;

// Minimum multiplier at the very edge (0 = no pheromone at edge)
static constexpr float MIN_BOUNDARY_MULTIPLIER = 0.1f;

ColonyPheromoneManager::ColonyPheromoneManager(float worldWidth, float worldHeight, float cellSize)
  : m_worldWidth(worldWidth)
  , m_worldHeight(worldHeight)
  , m_cellSize(cellSize) {
  // Pre-create grids for known teams
  m_homeGrids.emplace(TEAM_BLUE, PheromoneGrid(worldWidth, worldHeight, cellSize));
  m_homeGrids.emplace(TEAM_RED, PheromoneGrid(worldWidth, worldHeight, cellSize));
}

PheromoneGrid& ColonyPheromoneManager::GetHomeGrid(TeamId team) {
  auto it = m_homeGrids.find(team);
  if (it == m_homeGrids.end()) {
    // Create a new grid for this team
    m_homeGrids.emplace(team, PheromoneGrid(m_worldWidth, m_worldHeight, m_cellSize));
    return m_homeGrids.at(team);
  }
  return it->second;
}

const PheromoneGrid& ColonyPheromoneManager::GetHomeGrid(TeamId team) const {
  auto it = m_homeGrids.find(team);
  if (it == m_homeGrids.end()) {
    // Return blue grid as fallback (shouldn't happen in normal use)
    return m_homeGrids.at(TEAM_BLUE);
  }
  return it->second;
}

void ColonyPheromoneManager::DepositHome(TeamId team, const Vec2& position, float amount) {
  // Reduce pheromone intensity near world boundaries to discourage edge-hugging
  float boundaryMultiplier = GetBoundaryMultiplier(position);
  GetHomeGrid(team).Deposit(PHEROMONE_HOME, position, amount * boundaryMultiplier);
}

float ColonyPheromoneManager::GetHomeIntensity(TeamId team, const Vec2& position) const {
  if (!HasHomeGrid(team)) {
    return 0.0f;
  }
  return GetHomeGrid(team).GetIntensity(PHEROMONE_HOME, position);
}

void ColonyPheromoneManager::Update(float deltaTime) {
  for (auto& [team, grid] : m_homeGrids) {
    grid.Update(deltaTime);
  }
}

void ColonyPheromoneManager::ClearAll() {
  for (auto& [team, grid] : m_homeGrids) {
    grid.ClearAll();
  }
}

bool ColonyPheromoneManager::HasHomeGrid(TeamId team) const {
  return m_homeGrids.find(team) != m_homeGrids.end();
}

float ColonyPheromoneManager::GetBoundaryMultiplier(const Vec2& position) const {
  // Calculate distance to nearest boundary on each axis
  float distToLeft = position.x;
  float distToRight = m_worldWidth - position.x;
  float distToBottom = position.y;
  float distToTop = m_worldHeight - position.y;

  // Find minimum distance to any boundary
  float minDist = std::min({ distToLeft, distToRight, distToBottom, distToTop });

  // If outside fade zone, full intensity
  if (minDist >= BOUNDARY_FADE_DISTANCE) {
    return 1.0f;
  }

  // Linear fade from 1.0 at BOUNDARY_FADE_DISTANCE to MIN_BOUNDARY_MULTIPLIER at edge
  float t = minDist / BOUNDARY_FADE_DISTANCE;
  return MIN_BOUNDARY_MULTIPLIER + t * (1.0f - MIN_BOUNDARY_MULTIPLIER);
}

int ColonyPheromoneManager::GetCols() const {
  // All grids have same dimensions
  if (!m_homeGrids.empty()) {
    return m_homeGrids.begin()->second.GetCols();
  }
  return 0;
}

int ColonyPheromoneManager::GetRows() const {
  if (!m_homeGrids.empty()) {
    return m_homeGrids.begin()->second.GetRows();
  }
  return 0;
}

float ColonyPheromoneManager::GetCellIntensity(TeamId team, int cellX, int cellY) const {
  if (!HasHomeGrid(team)) {
    return 0.0f;
  }
  return GetHomeGrid(team).GetCellIntensity(PHEROMONE_HOME, cellX, cellY);
}

float ColonyPheromoneManager::GetMaxCellIntensity(TeamId team) const {
  if (!HasHomeGrid(team)) {
    return 0.0f;
  }
  return GetHomeGrid(team).GetMaxCellIntensity(PHEROMONE_HOME);
}
