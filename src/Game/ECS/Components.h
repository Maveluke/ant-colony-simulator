#pragma once
#include "Vec2.h"

// Components class

enum ComponentType {
	NONE = 0,
	PLAYER = 1 << 0,
	TRANSFORM = 1 << 1,
	AABB = 1 << 2,
	QUAD_RENDERER = 1 << 3,
	CIRCLE_RENDERER = 1 << 4
};


struct CPlayer {
	int score = 0;
	bool isLeftPlayer = true;
	float moveInput = 0.0f;
};


struct CTransform {
	Vec2 position = Vec2(0.0, 0.0);
	Vec2 velocity = Vec2(0.0, 0.0);
	Vec2 scale = Vec2(1.0, 1.0);
};


struct CAABB {
	Vec2 size = Vec2(0.0, 0.0);
	Vec2 halfSize = Vec2(0.0, 0.0);
};


// Renderer component
struct CQuadRenderer {
	Vec2 size = Vec2(1.0f, 1.0f);
	float z_depth = 0.0f;
	// Color as RGB values between 0 and 1 (default is white)
	float r = 1.0f, g = 1.0f, b = 1.0f;
};

struct CCircleRenderer {
	float radius = 1.0f;
	float z_depth = 0.0f;
	// Color as RGB values between 0 and 1 (default is white)
	float r = 1.0f, g = 1.0f, b = 1.0f;
};