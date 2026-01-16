#pragma once
#include <cmath>

class Vec2 {
public:
  float x = 0.0f;
  float y = 0.0f;

  Vec2() = default;
  Vec2(float x, float y) : x(x), y(y) {}

  // Arithmetic operators
  Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
  Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
  Vec2 operator*(float scale) const { return Vec2(x * scale, y * scale); }
  Vec2 operator/(float scale) const { return Vec2(x / scale, y / scale); }
  Vec2 operator-() const { return Vec2(-x, -y); }

  // Compound assignment
  Vec2& operator+=(const Vec2& other);
  Vec2& operator-=(const Vec2& other);
  Vec2& operator*=(float scale);
  Vec2& operator/=(float scale);

  // Comparison
  bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
  bool operator!=(const Vec2& other) const { return !(*this == other); }

  // Utility
  float Length() const;
  float LengthSquared() const;
  Vec2 Normalized() const;
  void Normalize();
  float Dot(const Vec2& other) const;
  float Cross(const Vec2& other) const;
  float Distance(const Vec2& other) const;
  float DistanceSquared(const Vec2& other) const;
  Vec2 Lerp(const Vec2& other, float t) const;
};

// Allows: scale * vec (not just vec * scale)
inline Vec2 operator*(float scale, const Vec2& v) { return v * scale; }