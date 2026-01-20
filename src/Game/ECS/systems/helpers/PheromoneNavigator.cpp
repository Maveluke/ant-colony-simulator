#include "ECS/systems/helpers/PheromoneNavigator.h"
#include "ECS/systems/helpers/RandomUtils.h"
#include <cmath>

namespace PheromoneNavigator {

  // =============================================================================
  // Direction Lookup Table (initialized once at startup)
  // =============================================================================

  DirectionTable::DirectionTable() {
    for (int i = 0; i < DIR_TABLE_SIZE; ++i) {
      // Spread across -PI to +PI
      float angle = (static_cast<float>(i) / DIR_TABLE_SIZE) * 2.0f * PI - PI;
      cosTable[i] = std::cos(angle);
      sinTable[i] = std::sin(angle);
    }
  }

  const DirectionTable& GetDirectionTable() {
    static const DirectionTable table;
    return table;
  }

  // =============================================================================
  // Sampling Implementation
  // =============================================================================

  Vec2 SampleBestDirection(
    const PheromoneGrid& pheromones,
    PheromoneType type,
    const Vec2& position,
    const Vec2& currentDirection,
    float coneAngle,
    float minIntensity,
    const Vec2* homePosition,
    float homeBias
  ) {
    float bestIntensity = 0.0f;
    Vec2 bestPosition = position;

    // Get base angle from current direction (still need atan2 once per call)
    float baseAngle = atan2(currentDirection.y, currentDirection.x);

    // Pre-compute base rotation (for rotating lookup table vectors)
    float baseCos = cos(baseAngle);
    float baseSin = sin(baseAngle);

    // Get the lookup table
    const DirectionTable& table = GetDirectionTable();

    // Calculate which portion of the table corresponds to the cone angle
    // coneAngle is half-angle, so we need entries within [-coneAngle, +coneAngle]
    // Table spans [-PI, +PI], so cone fraction = coneAngle / PI
    int coneEntries = static_cast<int>((coneAngle / PI) * (DIR_TABLE_SIZE / 2));
    if (coneEntries < 1) coneEntries = 1;
    int centerIndex = DIR_TABLE_SIZE / 2;  // Index for angle = 0

    for (int i = 0; i < NUM_SAMPLES; i++) {
      // Pick random offset within cone range
      int offset = RandomUtils::Int(-coneEntries, coneEntries);
      int tableIndex = (centerIndex + offset) % DIR_TABLE_SIZE;
      if (tableIndex < 0) tableIndex += DIR_TABLE_SIZE;

      // Get pre-computed direction offset
      float offsetCos = table.cosTable[tableIndex];
      float offsetSin = table.sinTable[tableIndex];

      // Rotate offset by base angle: 
      // final = base * offset (complex multiplication)
      // cos(a+b) = cos(a)cos(b) - sin(a)sin(b)
      // sin(a+b) = sin(a)cos(b) + cos(a)sin(b)
      float finalCos = baseCos * offsetCos - baseSin * offsetSin;
      float finalSin = baseSin * offsetCos + baseCos * offsetSin;

      float sampleDistance = RandomUtils::Float(SAMPLE_MIN_DISTANCE, SAMPLE_MAX_DISTANCE);

      Vec2 samplePos;
      samplePos.x = position.x + finalCos * sampleDistance;
      samplePos.y = position.y + finalSin * sampleDistance;

      float intensity = pheromones.GetIntensity(type, samplePos);

      if (intensity > bestIntensity) {
        bestIntensity = intensity;
        bestPosition = samplePos;
      }
    }

    if (bestIntensity > minIntensity) {
      Vec2 direction = bestPosition - position;

      // Optional: blend toward home position to keep on track
      if (homePosition != nullptr && homeBias > 0.0f) {
        Vec2 towardHome = *homePosition - position;
        direction += towardHome * homeBias;
      }

      float length = direction.Length();
      if (length > 0.001f) {
        return direction / length;
      }
    }

    return Vec2(0.0f, 0.0f);
  }
}