#include "RenderSystem.h"
#include "DrawUtils.h"

void MaveLib::RenderSystem::RenderQuad(const CTransform& transform, const CQuadRenderer& renderer) {
  DrawUtils::DrawRectangle(
    transform.position.x,
    transform.position.y,
    renderer.z_depth,
    renderer.size.x * transform.scale.x,
    renderer.size.y * transform.scale.y,
    renderer.r,
    renderer.g,
    renderer.b
  );
}
void MaveLib::RenderSystem::RenderCircle(const CTransform& transform, const CCircleRenderer& renderer) {
  DrawUtils::DrawCircle(
    transform.position.x,
    transform.position.y,
    renderer.z_depth,
    renderer.radius * transform.scale.x,
    renderer.radius * transform.scale.y,
    renderer.r,
    renderer.g,
    renderer.b
  );
}

void MaveLib::RenderSystem::Render(EntityManager& entityManager) {
  auto quad_entities = entityManager.GetEntitiesWithComponents(ComponentType::TRANSFORM | ComponentType::QUAD_RENDERER);
  for (const auto& entity : quad_entities) {
    const auto& transform = entityManager.GetComponent<CTransform>(entity);
    const auto& renderer = entityManager.GetComponent<CQuadRenderer>(entity);
    RenderQuad(transform, renderer);
  }

  auto circle_entities = entityManager.GetEntitiesWithComponents(ComponentType::TRANSFORM | ComponentType::CIRCLE_RENDERER);
  for (const auto& entity : circle_entities) {
    const auto& transform = entityManager.GetComponent<CTransform>(entity);
    const auto& renderer = entityManager.GetComponent<CCircleRenderer>(entity);
    RenderCircle(transform, renderer);
  }
}