#include "physics/manifold_factory.hpp"

#include <algorithm>
#include <cmath>

#include "physics/collider.hpp"
#include "physics/manifold.hpp"

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

    aPos += aToB.GetNormalized() * aRadius;
    bPos += bToA.GetNormalized() * bRadius;

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
        const core::Vec2f normal = aToB.y < 0.0f ? core::Vec2f(0.0f, 1.0f) : core::Vec2f(0.0f, -1.0f);
        return {normal, yOverlap};
    }

    // Point towards B knowing that aToB points from A to B
    const core::Vec2f normal = aToB.x < 0.0f ? core::Vec2f(1.0f, 0.0f) : core::Vec2f(-1.0f, 0.0f);
    return {normal, xOverlap};
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


    // Copy aToB to be the initial value of the closest point
    core::Vec2f clampedPoint;

    // Clamp point to the edge of the AABB
    clampedPoint.x = std::clamp(aabbToCircle.x, -scaledHWidth, scaledHWidth);
    clampedPoint.y = std::clamp(aabbToCircle.y, -scaledHHeight, scaledHHeight);


    // Put the point in "world space" because it was relative to the center
    const core::Vec2f closestPointOnAabb = aabbCenter + clampedPoint;

    core::Vec2f circleToClosestPoint = closestPointOnAabb - circleCenter;
    float distance = circleToClosestPoint.GetSqrMagnitude();
    if (distance > scaledRadius * scaledRadius) return Manifold::Empty();

    distance = std::sqrt(distance);

    core::Vec2f worldCircleToClosestPoint = circleToClosestPoint.NewMagnitude(scaledRadius) + circleCenter;

    if (std::abs(distance) >= 0.0001f)
    {
        circleToClosestPoint /= distance;
    }

    float depth = scaledRadius - distance;
    return {worldCircleToClosestPoint, closestPointOnAabb, circleToClosestPoint, depth};
}

Manifold algo::FindCircleAabbManifold(const CircleCollider* a, const Transform* ta, const AabbCollider* b,
                                      const Transform* tb)
{
    return FindAabbCircleManifold(b, tb, a, ta).Swaped();
}
}
