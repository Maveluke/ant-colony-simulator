#include "ECS/systems/helpers/PheromoneNavigator.h"
#include "ECS/systems/helpers/RandomUtils.h"
#include <cmath>

namespace PheromoneNavigator {

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

    float baseAngle = atan2(currentDirection.y, currentDirection.x);

    for (int i = 0; i < NUM_SAMPLES; i++) {
      float sampleAngle = baseAngle + RandomUtils::Float(-coneAngle, coneAngle);
      float sampleDistance = RandomUtils::Float(SAMPLE_MIN_DISTANCE, SAMPLE_MAX_DISTANCE);

      Vec2 samplePos;
      samplePos.x = position.x + cos(sampleAngle) * sampleDistance;
      samplePos.y = position.y + sin(sampleAngle) * sampleDistance;

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