#pragma once
#include "../Definition.h"
#include <vector>
#include <tuple>

// Collision Events
struct AntFoodCollision {
  Entity ant;
  Entity food;
};

struct AntColonyCollision {
  Entity ant;
  Entity colony;
};

struct FoodColonyCollision {
  Entity food;
  Entity colony;
};

struct AntSpiderCollision {
  Entity ant;
  Entity spider;
};

// Game Events
struct EntityDeathEvent {
  Entity entity;
};

struct FoodDepletedEvent {
  Entity food;
};

// Event Buffer
using EventBufferTuple = std::tuple<
  std::vector<AntFoodCollision>,
  std::vector<AntColonyCollision>,
  std::vector<FoodColonyCollision>,
  std::vector<AntSpiderCollision>,
  std::vector<EntityDeathEvent>,
  std::vector<FoodDepletedEvent>
>;

class EventBuffer {
private:
  EventBufferTuple m_events;

public:
  EventBuffer() = default;

  // Push an event of any registered type
  template<typename T>
  void Push(const T& event) {
    std::get<std::vector<T>>(m_events).push_back(event);
  }

  // Get all events of a specific type
  template<typename T>
  const std::vector<T>& Get() const {
    return std::get<std::vector<T>>(m_events);
  }

  void ClearAll() {
    std::get<std::vector<AntFoodCollision>>(m_events).clear();
    std::get<std::vector<AntColonyCollision>>(m_events).clear();
    std::get<std::vector<FoodColonyCollision>>(m_events).clear();
    std::get<std::vector<AntSpiderCollision>>(m_events).clear();
    std::get<std::vector<EntityDeathEvent>>(m_events).clear();
    std::get<std::vector<FoodDepletedEvent>>(m_events).clear();
  }
};