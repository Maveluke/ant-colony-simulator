#include "Camera.h"
#include <algorithm>
#include <cstdio>

namespace MaveLib {

  Camera::Camera(float screenW, float screenH, float worldW, float worldH)
    : screenWidth(screenW)
    , screenHeight(screenH)
    , worldWidth(worldW)
    , worldHeight(worldH)
    , zoom(1.0f)
    , minZoom(0.25f)
    , maxZoom(4.0f) {
    // Start camera centered on the world
    position = Vec2(worldW * 0.5f, worldH * 0.5f);
  }

  Vec2 Camera::WorldToScreen(const Vec2& worldPos) const {
    // 1. Translate: world position relative to camera center
    // 2. Scale by zoom
    // 3. Offset to screen center
    Vec2 relative = worldPos - position;
    Vec2 scaled = relative * zoom;
    Vec2 screenPos = scaled + Vec2(screenWidth * 0.5f, screenHeight * 0.5f);
    return screenPos;
  }

  Vec2 Camera::ScreenToWorld(const Vec2& screenPos) const {
    // Reverse of WorldToScreen
    Vec2 centered = screenPos - Vec2(screenWidth * 0.5f, screenHeight * 0.5f);
    Vec2 unscaled = centered / zoom;
    Vec2 worldPos = unscaled + position;
    return worldPos;
  }

  void Camera::GetViewBounds(float& minX, float& minY, float& maxX, float& maxY) const {
    // Calculate half-dimensions of visible area in world space
    float halfViewWidth = (screenWidth * 0.5f) / zoom;
    float halfViewHeight = (screenHeight * 0.5f) / zoom;

    minX = position.x - halfViewWidth;
    maxX = position.x + halfViewWidth;
    minY = position.y - halfViewHeight;
    maxY = position.y + halfViewHeight;
  }

  bool Camera::IsCircleVisible(const Vec2& center, float radius) const {
    float minX, minY, maxX, maxY;
    GetViewBounds(minX, minY, maxX, maxY);

    // Expand bounds by radius for proper circle intersection test
    return (center.x + radius >= minX &&
      center.x - radius <= maxX &&
      center.y + radius >= minY &&
      center.y - radius <= maxY);
  }

  bool Camera::IsRectVisible(const Vec2& center, float width, float height) const {
    float minX, minY, maxX, maxY;
    GetViewBounds(minX, minY, maxX, maxY);

    float halfW = width * 0.5f;
    float halfH = height * 0.5f;

    return (center.x + halfW >= minX &&
      center.x - halfW <= maxX &&
      center.y + halfH >= minY &&
      center.y - halfH <= maxY);
  }

  void Camera::Move(const Vec2& delta) {
    position += delta;
    ClampPosition();
  }

  void Camera::SetPosition(const Vec2& pos) {
    position = pos;
    ClampPosition();
  }

  void Camera::SetZoom(float newZoom) {
    zoom = newZoom;
    ClampZoom();
    ClampPosition(); // Re-clamp position since visible area changed
  }

  void Camera::AdjustZoom(float delta) {
    zoom += delta;
    ClampZoom();
    ClampPosition();
  }

  void Camera::SetZoomLimits(float min, float max) {
    minZoom = min;
    maxZoom = max;
    ClampZoom();
  }

  void Camera::SetWorldBounds(float width, float height) {
    worldWidth = width;
    worldHeight = height;
    ClampPosition();
  }

  void Camera::ClampPosition() {
    // Calculate half-dimensions of visible area
    float halfViewWidth = (screenWidth * 0.5f) / zoom;
    float halfViewHeight = (screenHeight * 0.5f) / zoom;

    // Clamp so the view doesn't go outside world bounds
    // If zoomed out so much that view is larger than world, center on world
    if (halfViewWidth * 2 >= worldWidth) {
      position.x = worldWidth * 0.5f;
    }
    else {
      position.x = std::clamp(position.x, halfViewWidth, worldWidth - halfViewWidth);
    }

    if (halfViewHeight * 2 >= worldHeight) {
      position.y = worldHeight * 0.5f;
    }
    else {
      position.y = std::clamp(position.y, halfViewHeight, worldHeight - halfViewHeight);
    }
  }

  void Camera::ClampZoom() {
    zoom = std::clamp(zoom, minZoom, maxZoom);
  }

}
