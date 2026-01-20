#pragma once
#include "graphics/Camera.h"

namespace DrawUtils {
  // ============================================
  // Camera-aware drawing functions (USE THESE)
  // ============================================

  // Draw a rectangle in world space (camera will transform to screen)
  void DrawRectangle(const MaveLib::Camera& camera,
    float worldX, float worldY, float z,
    float width, float height,
    float r, float g, float b);

  // Draw a circle in world space
  // segments: number of triangles (LOD - use fewer for distant/small objects)
  void DrawCircle(const MaveLib::Camera& camera,
    float worldX, float worldY, float z,
    float radiusX, float radiusY,
    float r, float g, float b,
    int segments = 8);

  // Draw a line in world space
  void DrawLine(const MaveLib::Camera& camera,
    float x1, float y1,
    float x2, float y2,
    float r, float g, float b);

  // Draw world border (the "table")
  void DrawWorldBorder(const MaveLib::Camera& camera,
    float worldWidth, float worldHeight,
    float thickness,
    float r, float g, float b);

  // ============================================
  // Screen-space drawing (for UI elements)
  // ============================================

  // Draw a rectangle in screen space (no camera transform)
  void DrawRectangleScreen(float screenX, float screenY, float z,
    float width, float height,
    float r, float g, float b);

  // Draw a debug grid in screen space
  void DrawDebugGrid(float cellSize, float r, float g, float b);

  // Print text at screen position
  void Print(float x, float y, const char* message,
    float r = 1.0f, float g = 1.0f, float b = 1.0f);
}