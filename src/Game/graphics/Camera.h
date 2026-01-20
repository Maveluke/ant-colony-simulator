#pragma once
#include "Vec2.h"

namespace MaveLib {

  class Camera {
  private:
    Vec2 position;          // Camera center in world space
    float zoom;             // 1.0 = normal, 2.0 = zoomed in, 0.5 = zoomed out
    float minZoom;
    float maxZoom;

    // Screen dimensions (needed for coordinate conversion)
    float screenWidth;
    float screenHeight;

    // World bounds (for clamping camera position)
    float worldWidth;
    float worldHeight;

  public:
    Camera(float screenW, float screenH, float worldW, float worldH);

    // Coordinate conversion
    Vec2 WorldToScreen(const Vec2& worldPos) const;
    Vec2 ScreenToWorld(const Vec2& screenPos) const;

    // Get the visible rectangle in world space (for frustum culling)
    // Returns: (minX, minY, maxX, maxY)
    void GetViewBounds(float& minX, float& minY, float& maxX, float& maxY) const;

    // Check if a circle is visible (for culling)
    bool IsCircleVisible(const Vec2& center, float radius) const;

    // Check if a rectangle is visible (for culling)
    bool IsRectVisible(const Vec2& center, float width, float height) const;

    // Movement and zoom
    void Move(const Vec2& delta);         // Move by delta (in world units)
    void SetPosition(const Vec2& pos);    // Set absolute position
    void SetZoom(float newZoom);
    void AdjustZoom(float delta);         // Add to current zoom

    // Getters
    Vec2 GetPosition() const { return position; }
    float GetZoom() const { return zoom; }
    float GetScreenWidth() const { return screenWidth; }
    float GetScreenHeight() const { return screenHeight; }

    // Setters for bounds
    void SetZoomLimits(float min, float max);
    void SetWorldBounds(float width, float height);

  private:
    void ClampPosition();   // Keep camera within world bounds
    void ClampZoom();
  };

}
