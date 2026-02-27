#include "gmath.h"

bool gmath_circle_vs_aabb(Vec2 center, float radius, Vec2 min, Vec2 max)
{
	float x = gmath_clamp(center.x, min.x, max.x);
	float y = gmath_clamp(center.y, min.y, max.y);

	Vec2 dist = gmath_vec2_sub(center, (Vec2) { x, y });

	return (dist.x * dist.x + dist.y * dist.y) <= radius * radius;
}