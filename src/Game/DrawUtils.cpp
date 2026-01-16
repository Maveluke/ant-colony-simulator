#include "DrawUtils.h"
#include "../ContestAPI/app.h"

void DrawUtils::DrawRectangle(float x, float y, float z, float width, float height, float r, float g, float b) {
  App::DrawTriangle(
    x - width / 2.0f, y - height / 2.0f, z, 1.0f,
    x + width / 2.0f, y - height / 2.0f, z, 1.0f,
    x + width / 2.0f, y + height / 2.0f, z, 1.0f,
    r, g, b, r, g, b, r, g, b
  );

  App::DrawTriangle(
    x - width / 2.0f, y - height / 2.0f, z, 1.0f,
    x + width / 2.0f, y + height / 2.0f, z, 1.0f,
    x - width / 2.0f, y + height / 2.0f, z, 1.0f,
    r, g, b, r, g, b, r, g, b
  );
}
void DrawUtils::DrawCircle(float x, float y, float z, float radiusX, float radiusY, float r, float g, float b) {
  int segments = 50;
  for (int i = 0; i <= segments; i++) {
    float angle = i * 2.0f * 3.1415926f / segments;
    float nextAngle = (i + 1) * 2.0f * 3.1415926f / segments;
    float x1 = x + cosf(angle) * radiusX;
    float y1 = y + sinf(angle) * radiusY;
    float x2 = x + cosf(nextAngle) * radiusX;
    float y2 = y + sinf(nextAngle) * radiusY;
    App::DrawTriangle(
      x, y, z, 1.0f,
      x1, y1, z, 1.0f,
      x2, y2, z, 1.0f,
      r, g, b, r, g, b, r, g, b
    );
  }
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
