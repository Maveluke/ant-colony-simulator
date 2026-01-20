#pragma once
#include "ECS/systems/grids/PheromoneGrid.h"
#include "ECS/Constants.h"
#include <cmath>

namespace PheromoneNavigator {

  constexpr int NUM_SAMPLES = 5;                 // Reduced from 8 for performance
  constexpr float SAMPLE_MIN_DISTANCE = 16.0f;
  constexpr float SAMPLE_MAX_DISTANCE = 48.0f;
  constexpr float SAMPLE_INTERVAL = 1.0f;        // Seconds between samples

  constexpr float WIDE_CONE_ANGLE = PI * 0.75f;  // ~135 degrees total
  constexpr float NARROW_CONE_ANGLE = PI / 4.0f; // ~45 degrees total

  constexpr float DEFAULT_MIN_INTENSITY = 0.1f;
  constexpr float DEFAULT_HOME_BIAS = 0.03f;

  // Lookup table size for pre-computed directions
  constexpr int DIR_TABLE_SIZE = 32;

  // =============================================================================
  // Pre-computed Direction Table (avoids sin/cos at runtime)
  // =============================================================================

  // Directions spread across a full circle
  // Usage: pick random index based on cone, rotate by base angle
  struct DirectionTable {
    float cosTable[DIR_TABLE_SIZE];
    float sinTable[DIR_TABLE_SIZE];

    DirectionTable();  // Initialized with actual sin/cos at startup
  };

  // Static lookup table (initialized once at program start)
  const DirectionTable& GetDirectionTable();

  // =============================================================================
  // Sampling Functions
  // =============================================================================

  // Monte Carlo pheromone sampling
  // Samples random positions in a cone and returns direction to the best one
  //
  // Parameters:
  //   pheromones      - The pheromone grid to sample from
  //   type            - Which pheromone type to sample
  //   position        - Current position to sample from
  //   currentDirection- Current facing direction (cone is centered on this)
  //   coneAngle       - Half-angle of sampling cone in radians
  //   minIntensity    - Minimum intensity to consider (filters noise)
  //   homePosition    - Optional: if provided, blends result toward home
  //   homeBias        - How much to blend toward home (0.0 to 1.0)
  //
  // Returns:
  //   Normalized direction toward best sample, or zero vector if none found
  Vec2 SampleBestDirection(
    const PheromoneGrid& pheromones,
    PheromoneType type,
    const Vec2& position,
    const Vec2& currentDirection,
    float coneAngle,
    float minIntensity = DEFAULT_MIN_INTENSITY,
    const Vec2* homePosition = nullptr,
    float homeBias = DEFAULT_HOME_BIAS
  );
}