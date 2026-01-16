#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components.h"

namespace MaveLib {
  /*
    Render System
  */
  class RenderSystem {
  public:
    RenderSystem() = default;
    ~RenderSystem() = default;

    void Render(EntityManager& entityManager);

  private:
    void RenderQuad(const CTransform& transform, const CQuadRenderer& renderer);
    void RenderCircle(const CTransform& transform, const CCircleRenderer& renderer);
  };
}