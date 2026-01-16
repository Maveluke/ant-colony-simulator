#include "Vec2.h"

Vec2& Vec2::operator+=(const Vec2& other) {
  x += other.x;
  y += other.y;
  return *this;
}

Vec2& Vec2::operator-=(const Vec2& other) {
  x -= other.x;
  y -= other.y;
  return *this;
}

Vec2& Vec2::operator*=(float scale) {
  x *= scale;
  y *= scale;
  return *this;
}

Vec2& Vec2::operator/=(float scale) {
  x /= scale;
  y /= scale;
  return *this;
}

float Vec2::Length() const {
  return std::sqrt(x * x + y * y);
}

float Vec2::LengthSquared() const {
  return x * x + y * y;
}

Vec2 Vec2::Normalized() const {
  float len = Length();
  if (len > 0.0f) {
    return Vec2(x / len, y / len);
  }
  return Vec2();
}

void Vec2::Normalize() {
  float len = Length();
  if (len > 0.0f) {
    x /= len;
    y /= len;
  }
}

float Vec2::Dot(const Vec2& other) const {
  return x * other.x + y * other.y;
}

float Vec2::Cross(const Vec2& other) const {
  return x * other.y - y * other.x;
}

float Vec2::Distance(const Vec2& other) const {
  return (*this - other).Length();
}

float Vec2::DistanceSquared(const Vec2& other) const {
  return (*this - other).LengthSquared();
}

Vec2 Vec2::Lerp(const Vec2& other, float t) const {
  return *this + (other - *this) * t;
}