# Ant Colony Simulation - Technical Documentation

## Game Overview

**Ant Colony Simulation** is a real-time strategy/simulation game where two rival ant colonies compete for resources. Each ant is an individually simulated entity with its own AI state machine, navigating using pheromone trails—just like real ants.

The game fits the **"Simulation of Large Number of Entities"** challenge theme by:
- Simulating **thousands of ants** simultaneously, each with independent behavior
- Using **emergent gameplay** where complex colony behavior arises from simple individual rules
- Implementing **biologically-inspired pheromone communication** for pathfinding
- Featuring **inter-colony warfare** with swarm combat mechanics

### What Can Players Do?

- **Spawn Food** - Place food sources and watch ants discover, compete, and form trails
- **Spawn Spiders** - Introduce predators that trigger alarm pheromones and swarm combat
- **Draw Pheromone Trails** - Paint food pheromone paths to guide ants manually
- **Visualize Pheromones** - Toggle debug overlays to see home trails, food trails, and alarm signals

### Core Gameplay Loop
1. Colonies spawn ants (fueled by stored food)
2. Ants wander, searching for food or following pheromone trails
3. When food is found, ants drag it back, leaving trails for others
4. Rival ants and spiders create combat encounters
5. The colony with better resource management thrives

---

## Controls

| Input | Action |
|-------|--------|
| **Left Click (Hold)** | Spawn/draw current selection at cursor |
| **WASD** | Move the camera |

**Spawn Mode (Pheromone View OFF):**
- **K** = Spawn Food | **J** = Spawn Spider | **I** = Draw Food Pheromone | **9** = Toggle view ON

**Debug Mode (Pheromone View ON):**
- **K** = Toggle HOME | **J** = Toggle FOOD | **I** = Toggle ALARM | **9** = Toggle view OFF

---

## Architecture Overview

```
GameEngine → ScenePlay → [EntityManager, SpatialGrid, PheromoneGrids, RenderSystem]
                              ↓
              Systems: AntSystem, SpiderSystem, CollisionSystem, 
                       MovementSystem, DragSystem
```

**Frame Update Order:**
1. Rebuild SpatialGrid with current positions
2. AntSystem/SpiderSystem AI updates + pheromone deposits
3. CollisionSystem detects → EventBuffer stores events
4. AntCollisionSystem processes collision events
5. DragSystem + MovementSystem apply velocities
6. PheromoneGrids decay, EntityManager processes add/delete
7. RenderSystem draws with frustum culling

---

## Core Systems

### 1. Entity Component System (ECS)

**Key Design:** Cache-friendly memory pool where all components of the same type are stored contiguously. Entity ID is the index into each component vector, maximizing CPU cache hits during iteration.

**Bitmask Lookups:** Component existence is checked via bitwise AND on a uint32 mask—O(1) instead of map lookups.

**Cached Entity Lists:** Instead of filtering 10,000 entities every frame to find ants, we cache `std::vector<Entity>` for each entity type and rebuild only when entities are added/removed.

**Action System (Observer Pattern):** Raw input is decoupled from game logic. `GameEngine` polls hardware, maps inputs to named actions ("SPAWN", "TOGGLE_VIEW"), and scenes respond only to registered actions—enabling easy rebinding and multi-input support.

---

### 2. Spatial Partitioning

**SpatialGrid** divides the world into cells (32×32 units). Essential for O(1) neighbor queries instead of O(n²) all-pairs checks.

**Zero-Allocation Queries:** Instead of returning `std::vector<Entity>`, we use a template callback pattern:
```cpp
grid.QueryEach(position, radius, [&](Entity e) { /* process inline */ });
```
This eliminates thousands of heap allocations per frame.

**Combined Query:** Each ant needs nearest spider, enemy ant, AND food. Instead of 3 queries, `NearbyQuery::QueryAll()` returns all three in a single pass—66% fewer queries.

---

### 3. Pheromone System

Three pheromone types on grid layers (16×16 cells for smooth gradients):
- **HOME** - Per-team separate grids so ants return to their own colony
- **FOOD** - Shared trail marking paths to food sources
- **ALARM** - Coordinates swarm attacks against threats

**Monte Carlo Sampling:** Instead of expensive gradient computation (sampling 25+ neighbors), ants sample 5 random positions in a forward cone and move toward the highest intensity. This creates natural path variation and prevents traffic jams.

**Pre-computed Trig Tables:** Direction calculations use lookup tables instead of runtime sin/cos.

**Staggered Decay:** Pheromone layers decay on different timers, distributing work across frames.

---

### 4. Ant AI State Machine

Five states: **WANDER** → **FOLLOW_TRAIL** → **FORAGE** → (combat: **ATTACK** / **FLEE**)

- **WANDER**: Explore randomly, detect food trails or threats
- **FOLLOW_TRAIL**: Navigate toward food using pheromone sampling
- **FORAGE**: Carry food home while depositing food pheromone trails
- **ATTACK**: Move toward threats, deposit alarm pheromone to attract allies
- **FLEE**: Retreat from danger, then return to WANDER

**AntContext Bundling:** All component references and spatial query results are fetched once per ant per frame into a context struct, avoiding repeated lookups in each state handler.

**Stuck Detection:** Track position history; if ant hasn't moved enough, trigger random escape direction to break out of traffic jams or obstacles.

---

### 5. Collision & Event System

**Event Buffer Pattern:** Collision detection pushes typed events (AntFoodCollision, AntSpiderCollision, etc.) into vectors. After detection completes, a separate pass processes events. This prevents modification-during-iteration bugs.

**Collision Responses:**
- Ant + Food → Start dragging, transition to FORAGE
- Food + Colony → Deposit food, delete food entity
- Ant + Spider → Combat damage, alarm pheromone

---

### 6. Drag System (Cooperative Carrying)

Multiple ants can drag heavy food. `CDraggable` tracks weight and current draggers.

**Unified Group Movement:** Instead of each dragger sampling pheromones independently (causing pull-apart), we sample once from the food's position and apply the same direction to all draggers.

---

### 7. Rendering

**Frustum Culling:** `Camera.IsCircleVisible()` skips rendering for off-screen entities.

**Coordinate Conversion:** Camera handles world↔screen transforms with pan/zoom support.

---

## Performance Summary

| Optimization | Impact |
|-------------|--------|
| Contiguous ECS Memory Pool | Cache-friendly iteration |
| Cached Entity Lists | O(1) vs O(n) entity filtering |
| Zero-Allocation Spatial Queries | No heap allocs per query |
| Combined NearbyQuery | 3 results in 1 query |
| Monte Carlo Sampling | 5 samples vs 25+ gradient |
| Pre-computed Trig Tables | No sin/cos at runtime |
| Staggered Pheromone Decay | Distributed frame work |
| AntContext Bundling | Fetch components once |
| Event Buffer Pattern | Safe iteration |
| Frustum Culling | Skip off-screen entities |

---

## Future Expansion Ideas

- **Archetype ECS** - Group entities by component signature for even better cache locality
- **Quadtree** - Variable-density spatial partitioning
- **SIMD** - Vectorized pheromone decay and position updates
- **Level of Detail** - Simplified AI for off-screen ants
