#pragma once
#include <cstdlib>

namespace RandomUtils {

  // Generate random float between min and max
  inline float Float(float min, float max) {
    return min + (rand() / static_cast<float>(RAND_MAX)) * (max - min);
  }

  // Generate random bool with given probability of true (0.0 to 1.0)
  inline bool Bool(float probability = 0.5f) {
    return Float(0.0f, 1.0f) < probability;
  }
}