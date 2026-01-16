#pragma once

namespace DrawUtils {
  // Draw a rectangle centered at (x, y) with given width, height and color (r, g, b) with z-depth
  void DrawRectangle(float x, float y, float z, float width, float height, float r, float g, float b);
  // Draw a circle centered at (x, y) with given radius and color (r, g, b) with z-depth
  void DrawCircle(float x, float y, float z, float radiusX, float radiusY, float r, float g, float b);
  // Draw a debug grid with given cell size and color (r, g, b)
  void DrawDebugGrid(float cellSize, float r, float g, float b);
}