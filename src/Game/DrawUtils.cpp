#include "DrawUtils.h"
#include "../ContestAPI/app.h"

// ============================================
// Camera-aware drawing functions
// ============================================

void DrawUtils::DrawRectangle(const MaveLib::Camera& camera,
  float worldX, float worldY, float z,
  float width, float height,
  float r, float g, float b) {

  // Transform world position to screen position
  Vec2 screenPos = camera.WorldToScreen(Vec2(worldX, worldY));

  // Scale dimensions by zoom
  float zoom = camera.GetZoom();
  float scaledWidth = width * zoom;
  float scaledHeight = height * zoom;

  // Draw using screen coordinates
  float x = screenPos.x;
  float y = screenPos.y;

  App::DrawTriangle(
    x - scaledWidth / 2.0f, y - scaledHeight / 2.0f, z, 1.0f,
    x + scaledWidth / 2.0f, y - scaledHeight / 2.0f, z, 1.0f,
    x + scaledWidth / 2.0f, y + scaledHeight / 2.0f, z, 1.0f,
    r, g, b, r, g, b, r, g, b
  );

  App::DrawTriangle(
    x - scaledWidth / 2.0f, y - scaledHeight / 2.0f, z, 1.0f,
    x + scaledWidth / 2.0f, y + scaledHeight / 2.0f, z, 1.0f,
    x - scaledWidth / 2.0f, y + scaledHeight / 2.0f, z, 1.0f,
    r, g, b, r, g, b, r, g, b
  );
}

void DrawUtils::DrawCircle(const MaveLib::Camera& camera,
  float worldX, float worldY, float z,
  float radiusX, float radiusY,
  float r, float g, float b) {

  Vec2 screenPos = camera.WorldToScreen(Vec2(worldX, worldY));
  float zoom = camera.GetZoom();
  float scaledRadiusX = radiusX * zoom;
  float scaledRadiusY = radiusY * zoom;

  float x = screenPos.x;
  float y = screenPos.y;

  int segments = 50;
  for (int i = 0; i <= segments; i++) {
    float angle = i * 2.0f * 3.1415926f / segments;
    float nextAngle = (i + 1) * 2.0f * 3.1415926f / segments;
    float x1 = x + cosf(angle) * scaledRadiusX;
    float y1 = y + sinf(angle) * scaledRadiusY;
    float x2 = x + cosf(nextAngle) * scaledRadiusX;
    float y2 = y + sinf(nextAngle) * scaledRadiusY;
    App::DrawTriangle(
      x, y, z, 1.0f,
      x1, y1, z, 1.0f,
      x2, y2, z, 1.0f,
      r, g, b, r, g, b, r, g, b
    );
  }
}

void DrawUtils::DrawLine(const MaveLib::Camera& camera,
  float x1, float y1,
  float x2, float y2,
  float r, float g, float b) {

  Vec2 screen1 = camera.WorldToScreen(Vec2(x1, y1));
  Vec2 screen2 = camera.WorldToScreen(Vec2(x2, y2));

  App::DrawLine(screen1.x, screen1.y, screen2.x, screen2.y, r, g, b);
}

void DrawUtils::DrawWorldBorder(const MaveLib::Camera& camera,
  float worldWidth, float worldHeight,
  float thickness,
  float r, float g, float b) {

  // Draw 4 border rectangles around the world edges
  // Bottom border
  DrawRectangle(camera,
    worldWidth * 0.5f, -thickness * 0.5f, 0.0f,
    worldWidth + thickness * 2, thickness,
    r, g, b);

  // Top border
  DrawRectangle(camera,
    worldWidth * 0.5f, worldHeight + thickness * 0.5f, 0.0f,
    worldWidth + thickness * 2, thickness,
    r, g, b);

  // Left border
  DrawRectangle(camera,
    -thickness * 0.5f, worldHeight * 0.5f, 0.0f,
    thickness, worldHeight + thickness * 2,
    r, g, b);

  // Right border
  DrawRectangle(camera,
    worldWidth + thickness * 0.5f, worldHeight * 0.5f, 0.0f,
    thickness, worldHeight + thickness * 2,
    r, g, b);
}

// ============================================
// Screen-space drawing (for UI)
// ============================================

void DrawUtils::DrawRectangleScreen(float screenX, float screenY, float z,
  float width, float height,
  float r, float g, float b) {

  App::DrawTriangle(
    screenX - width / 2.0f, screenY - height / 2.0f, z, 1.0f,
    screenX + width / 2.0f, screenY - height / 2.0f, z, 1.0f,
    screenX + width / 2.0f, screenY + height / 2.0f, z, 1.0f,
    r, g, b, r, g, b, r, g, b
  );

  App::DrawTriangle(
    screenX - width / 2.0f, screenY - height / 2.0f, z, 1.0f,
    screenX + width / 2.0f, screenY + height / 2.0f, z, 1.0f,
    screenX - width / 2.0f, screenY + height / 2.0f, z, 1.0f,
    r, g, b, r, g, b, r, g, b
  );
}

void DrawUtils::DrawDebugGrid(float cellSize, float r, float g, float b) {
  float startX = -1.0f;
  float endX = 1.0f;
  float startY = -1.0f;
  float endY = 1.0f;
#ifdef APP_USE_VIRTUAL_RES
  APP_NATIVE_TO_VIRTUAL_COORDS(startX, startY);
  APP_NATIVE_TO_VIRTUAL_COORDS(endX, endY);
#endif
  // Vertical lines
  for (float x = startX; x <= endX; x += cellSize) {
    App::DrawLine(x, startY, x, endY, r, g, b);
  }
  // Horizontal lines
  for (float y = startY; y <= endY; y += cellSize) {
    App::DrawLine(startX, y, endX, y, r, g, b);
  }
}

void DrawUtils::Print(float x, float y, const char* message, float r, float g, float b) {
  App::Print(x, y, message, r, g, b);
}