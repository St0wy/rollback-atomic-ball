#include "physics/manifold_factory.hpp"

#include <algorithm>
#include <cmath>

#include "physics/collider.hpp"
#include "physics/manifold.hpp"
#include "maths/basic.hpp"

namespace game
{
Manifold algo::FindCircleCircleManifold(
	const CircleCollider* a, const Transform* ta,
	const CircleCollider* b, const Transform* tb)
{
	core::Vec2f aPos = a->center + ta->position;
	core::Vec2f bPos = b->center + tb->position;

	const float aRadius = a->radius * ta->scale.Major();
	const float bRadius = b->radius * tb->scale.Major();

	const core::Vec2f aToB = bPos - aPos;
	const core::Vec2f bToA = aPos - bPos;

	if (aToB.GetMagnitude() > aRadius + bRadius)
	{
		return Manifold::Empty();
	}

	aPos += aToB.NewMagnitude(aRadius);
	bPos += bToA.NewMagnitude(bRadius);

	const core::Vec2f collisionPointsDistance = bPos - aPos;

	return {
		aPos,
		bPos,
		collisionPointsDistance.GetNormalized(),
		collisionPointsDistance.GetMagnitude()
	};
}

Manifold algo::FindAabbAabbManifold(
	const AabbCollider* a, const Transform* ta,
	const AabbCollider* b, const Transform* tb)
{
	const core::Vec2f transformedCenterA = ta->position + a->center;
	const float aScaledHWidth = a->halfWidth * ta->scale.x;
	const float aScaledHHeight = a->halfHeight * ta->scale.y;

	const core::Vec2f transformedCenterB = tb->position + b->center;
	const float bScaledHWidth = b->halfWidth * tb->scale.x;
	const float bScaledHHeight = b->halfHeight * tb->scale.y;

	const core::Vec2f aToB = transformedCenterB - transformedCenterA;
	const float xOverlap = aScaledHWidth + bScaledHWidth - std::abs(aToB.x);

	// Overlap test on x axis
	if (xOverlap <= 0.0f) return Manifold::Empty();

	const float yOverlap = aScaledHHeight + bScaledHHeight - std::abs(aToB.y);

	// Overlap test on y axis
	if (yOverlap <= 0.0f) return Manifold::Empty();

	// Find out which axis is axis of least penetration
	if (xOverlap > yOverlap)
	{
		// Point towards B knowing that aToB points from A to B
		core::Vec2f normal;

		float aY;
		float bY;
		if (aToB.y > 0.0f)
		{
			normal = core::Vec2f(0.0f, -1.0f);
			aY = transformedCenterA.y + aScaledHHeight;
			bY = transformedCenterB.y - bScaledHHeight;
		}
		else
		{
			normal = core::Vec2f(0.0f, 1.0f);
			aY = transformedCenterA.y - aScaledHHeight;
			bY = transformedCenterB.y + bScaledHHeight;
		}

		float x;
		if (aToB.x > 0.0f)
		{
			x = transformedCenterA.x + aScaledHWidth - xOverlap / 2.0f;
		}
		else
		{
			x = transformedCenterA.x - aScaledHWidth + xOverlap / 2.0f;
		}

		return { {x, aY}, {x, bY}, normal, yOverlap };
	}

	// Point towards B knowing that aToB points from A to B
	core::Vec2f normal;

	float y;
	if (aToB.y > 0.0f)
	{
		y = transformedCenterA.x + aScaledHWidth - xOverlap / 2.0f;
	}
	else
	{
		y = transformedCenterA.x - aScaledHWidth + xOverlap / 2.0f;
	}

	float aX;
	float bX;
	if (aToB.x > 0.0f)
	{
		normal = core::Vec2f(-1.0f, 0.0f);
		aX = transformedCenterA.x + aScaledHWidth;
		bX = transformedCenterB.x - bScaledHWidth;
	}
	else
	{
		normal = core::Vec2f(1.0f, 0.0f);
		aX = transformedCenterA.x - aScaledHWidth;
		bX = transformedCenterB.x + bScaledHWidth;
	}
	return { {aX, y}, {bX, y}, normal, xOverlap };
}

Manifold algo::FindAabbCircleManifold(
	const AabbCollider* a, const Transform* ta,
	const CircleCollider* b, const Transform* tb)
{
	// Apply the transform to the AabbCollider
	const core::Vec2f aabbCenter = ta->position + a->center;
	const float scaledHWidth = a->halfWidth * ta->scale.x;
	const float scaledHHeight = a->halfHeight * ta->scale.y;

	// Apply the transform to the circle collider
	const core::Vec2f circleCenter = tb->position + b->center;
	const float scaledRadius = b->radius * tb->scale.Major();

	const core::Vec2f aabbToCircle = circleCenter - aabbCenter;

	core::Vec2f clampedPoint;

	// Clamp point to the edge of the AABB
	clampedPoint.x = std::clamp(aabbToCircle.x, -scaledHWidth, scaledHWidth);
	clampedPoint.y = std::clamp(aabbToCircle.y, -scaledHHeight, scaledHHeight);

	bool isCircleCenterInside = false;

	// If they're equal, the center of the circle is inside the AABB
	if (clampedPoint == aabbToCircle)
	{
		isCircleCenterInside = true;

		// We still want one point on the side of the AABB, so we find the nearest border, and clamp on it
		const float distToPosWidth = std::abs(scaledHWidth - clampedPoint.x);
		const float distToNegWidth = std::abs(-scaledHWidth - clampedPoint.x);
		const float distToPosHeight = std::abs(scaledHHeight - clampedPoint.y);
		const float distToNegHeight = std::abs(-scaledHHeight - clampedPoint.y);

		const float smallest = std::min({ distToPosWidth, distToNegWidth, distToPosHeight, distToNegHeight });

		if (smallest == distToPosWidth) // NOLINT(clang-diagnostic-float-equal)
		{
			clampedPoint.x = scaledHWidth;
		}
		else if (smallest == distToNegWidth) // NOLINT(clang-diagnostic-float-equal)
		{
			clampedPoint.x = -scaledHWidth;
		}
		else if (smallest == distToPosHeight) // NOLINT(clang-diagnostic-float-equal)
		{
			clampedPoint.y = scaledHHeight;
		}
		else if (smallest == distToNegHeight) // NOLINT(clang-diagnostic-float-equal)
		{
			clampedPoint.y = -scaledHHeight;
		}
	}

	// Put the point in "world space" because it was relative to the center
	const core::Vec2f closestPointOnAabb = aabbCenter + clampedPoint;

	const core::Vec2f circleToClosestPoint = closestPointOnAabb - circleCenter;

	// Distance between the circle center and the clamped point
	const float squaredDistance = circleToClosestPoint.GetSqrMagnitude();

	if (!isCircleCenterInside && squaredDistance >= scaledRadius * scaledRadius) return Manifold::Empty();

	// This is the collision point around the circle
	core::Vec2f aroundCirclePoint = circleToClosestPoint.NewMagnitude(scaledRadius);
	if (isCircleCenterInside)
	{
		aroundCirclePoint = -aroundCirclePoint;
	}

	core::Vec2f worldAroundCirclePoint = aroundCirclePoint + circleCenter;

	const core::Vec2f diff = closestPointOnAabb - worldAroundCirclePoint;

	return { worldAroundCirclePoint, closestPointOnAabb, diff.GetNormalized(), diff.GetMagnitude() };
}

Manifold algo::FindCircleAabbManifold(const CircleCollider* a, const Transform* ta, const AabbCollider* b,
	const Transform* tb)
{
	return FindAabbCircleManifold(b, tb, a, ta).Swaped();
}
}
