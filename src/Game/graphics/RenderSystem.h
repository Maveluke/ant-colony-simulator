#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components.h"
#include "Camera.h"

namespace MaveLib {
  /*
    Render System - renders entities with camera support and frustum culling
  */
  class RenderSystem {
  public:
    RenderSystem() = default;
    ~RenderSystem() = default;

    // Render all visible entities through the camera
    void Render(EntityManager& entityManager, const Camera& camera);

  private:
    void RenderQuad(const CTransform& transform, const CQuadRenderer& renderer, const Camera& camera);
    void RenderCircle(const CTransform& transform, const CCircleRenderer& renderer, const Camera& camera);
  };
}