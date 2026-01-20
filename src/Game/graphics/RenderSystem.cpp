#include "RenderSystem.h"
#include "DrawUtils.h"

void MaveLib::RenderSystem::RenderQuad(const CTransform& transform,
  const CQuadRenderer& renderer,
  const Camera& camera) {

  float width = renderer.size.x * transform.scale.x;
  float height = renderer.size.y * transform.scale.y;

  // Frustum culling: skip if not visible
  if (!camera.IsRectVisible(transform.position, width, height)) {
    return;
  }

  DrawUtils::DrawRectangle(
    camera,
    transform.position.x,
    transform.position.y,
    renderer.z_depth,
    width,
    height,
    renderer.r,
    renderer.g,
    renderer.b
  );
}

void MaveLib::RenderSystem::RenderCircle(const CTransform& transform,
  const CCircleRenderer& renderer,
  const Camera& camera) {

  float radiusX = renderer.radius * transform.scale.x;
  float radiusY = renderer.radius * transform.scale.y;
  float maxRadius = (radiusX > radiusY) ? radiusX : radiusY;

  // Frustum culling: skip if not visible
  if (!camera.IsCircleVisible(transform.position, maxRadius)) {
    return;
  }

  // LOD: Calculate screen-space radius for segment count
  float zoom = camera.GetZoom();
  float screenRadius = maxRadius * zoom;

  // Choose segments based on screen size
  int segments;
  if (screenRadius < 3.0f) {
    segments = 4;   // Tiny - diamond shape
  }
  else if (screenRadius < 8.0f) {
    segments = 6;   // Small - hexagon
  }
  else if (screenRadius < 20.0f) {
    segments = 8;   // Medium - octagon
  }
  else {
    segments = 12;  // Large - smooth circle
  }

  DrawUtils::DrawCircle(
    camera,
    transform.position.x,
    transform.position.y,
    renderer.z_depth,
    radiusX,
    radiusY,
    renderer.r,
    renderer.g,
    renderer.b,
    segments
  );
}

void MaveLib::RenderSystem::Render(EntityManager& entityManager, const Camera& camera) {
  // Render quads
  auto quad_entities = entityManager.GetEntitiesWithComponents(
    ComponentType::TRANSFORM | ComponentType::QUAD_RENDERER);

  for (const auto& entity : quad_entities) {
    const auto& transform = entityManager.GetComponent<CTransform>(entity);
    const auto& renderer = entityManager.GetComponent<CQuadRenderer>(entity);
    RenderQuad(transform, renderer, camera);
  }

  // Render circles
  auto circle_entities = entityManager.GetEntitiesWithComponents(
    ComponentType::TRANSFORM | ComponentType::CIRCLE_RENDERER);

  for (const auto& entity : circle_entities) {
    const auto& transform = entityManager.GetComponent<CTransform>(entity);
    const auto& renderer = entityManager.GetComponent<CCircleRenderer>(entity);
    RenderCircle(transform, renderer, camera);
  }
}