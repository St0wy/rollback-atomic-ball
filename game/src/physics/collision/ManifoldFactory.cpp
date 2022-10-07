#include "physics/collision/ManifoldFactory.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "physics/collision/Collider.hpp"
#include "physics/collision/Manifold.hpp"
#include "physics/collision/Simplex.hpp"

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

Manifold algo::FindCircleBoxManifold(
    const CircleCollider* a,
    const Transform* ta,
    const BoxCollider* b,
    const Transform* tb
)
{
    return FindBoxCircleManifold(b, tb, a, ta);
}

Manifold algo::FindBoxCircleManifold(const BoxCollider* a, const Transform* ta, const CircleCollider* b,
                                     const Transform* tb)
{
    return Gjk(a, ta, b, tb);
}

Manifold algo::FindBoxBoxManifold(
    const BoxCollider* a,
    const Transform* ta,
    const BoxCollider* b,
    const Transform* tb
)
{
    return Gjk(a, ta, b, tb);
    //return Sat(a, ta, b, tb);
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

core::Vec2f algo::Support(
    const Collider* colliderA,
    const Transform* transformA,
    const Collider* colliderB,
    const Transform* transformB,
    const core::Vec2f& direction
)
{
    return colliderA->FindFurthestPoint(transformA, direction) -
        colliderB->FindFurthestPoint(transformB, -direction);
}

Manifold algo::Gjk(
    const Collider* colliderA,
    const Transform* transformA,
    const Collider* colliderB,
    const Transform* transformB
)
{
    core::Vec2f direction = core::Vec2f::Normalize(transformB->position - transformA->position);

    core::Vec2f support = Support(
        colliderA,
        transformA,
        colliderB,
        transformB,
        direction);

    Simplex points;
    points.PushFront(support);

    // New direction is towards the origin
    direction = core::Vec2f::Normalize(-support);

    while (true)
    {
        support = Support(colliderA, transformA, colliderB, transformB, direction);

        if (support.Dot(direction) <= 0)
        {
            return Manifold::Empty();
        }

        points.PushFront(support);

        if (NextSimplex(points, direction))
        {
            return Epa(points, colliderA, transformA, colliderB, transformB);
        }
    }
}

bool algo::NextSimplex(const Simplex& points, core::Vec2f& direction)
{
    switch (points.Size())
    {
    case 2:
        return Line(points, direction);
    case 3:
        return Triangle(points, direction);
    default:
        return false;
    }
}

bool algo::SameDirection(const core::Vec2f direction, const core::Vec2f ao)
{
    return direction.Dot(ao) > 0;
}

bool algo::Line(const Simplex& points, core::Vec2f& direction)
{
    const core::Vec2f a = points[0];
    const core::Vec2f b = points[1];
    const core::Vec2f ab = core::Vec2f::Normalize(b - a);
    const core::Vec2f ao = core::Vec2f::Normalize(-a);
    direction = core::Vec2f::TripleProduct(ab, ao, ab);

    return false;
}

bool algo::Triangle(const Simplex& points, core::Vec2f& direction)
{
    const core::Vec2f a = points[0];
    const core::Vec2f b = points[1];
    const core::Vec2f c = points[2];

    const core::Vec2f ab = core::Vec2f::Normalize(b - a);
    const core::Vec2f ac = core::Vec2f::Normalize(c - a);
    const core::Vec2f ao = core::Vec2f::Normalize(-a);

    //const core::Vec2f tripple = TripleProduct(ac, ac, ab);
    const core::Vec2f abf = core::Vec2f::TripleProduct(ac, ab, ab);
    const core::Vec2f acf = core::Vec2f::TripleProduct(ab, ac, ac);

    if (SameDirection(abf, ao))
    {
        direction = abf;
        return false;
        //return Line(points = { a, b }, direction);
    }
    if (SameDirection(acf, ao))
    {
        direction = acf;
        return false;
        //return Line(points = { a, c }, direction);
    }
    return true;
}

Manifold algo::Epa(
    const Simplex& simplex,
    const Collider* colliderA,
    const Transform* transformA,
    const Collider* colliderB,
    const Transform* transformB
)
{
    std::vector polytope(simplex.Begin(), simplex.End());

    core::Vec2f minNormal;
    float minDistance = std::numeric_limits<float>::infinity();
    std::size_t minIndex = 0;

    std::size_t iterations = 0;
    constexpr std::size_t maxIter = 30;
    while (minDistance == std::numeric_limits<float>::infinity()) // NOLINT(clang-diagnostic-float-equal)
    {
        if (iterations++ > maxIter)
        {
            break;
        }

        for (std::size_t i = 0; i < polytope.size(); i++)
        {
            const std::size_t j = (i + 1) % polytope.size();

			const core::Vec2f vertexI = polytope[i];
            core::Vec2f vertexJ = polytope[j];

            core::Vec2f ij = vertexJ - vertexI;

            core::Vec2f normal = ij.NegativePerpendicular().GetNormalized();
            float distance = normal.Dot(vertexI);

            if (distance < 0.0f)
            {
                distance *= -1.0f;
                normal *= -1.0f;
            }

            if (distance < minDistance)
            {
                minDistance = distance;
                minNormal = normal;
                minIndex = i;
            }
        }

        core::Vec2f support = Support(colliderA, transformA, colliderB, transformB, minNormal);
        const float sDistance = minNormal.Dot(support);

        if (std::abs(sDistance - minDistance) > 0.001f)
        {
            minDistance = std::numeric_limits<float>::infinity();
            const auto convIndex = static_cast<long long>(minIndex);
            polytope.insert(polytope.begin() + convIndex + 1ll, support);
        }
    }

    if (minDistance == std::numeric_limits<float>::infinity()) // NOLINT(clang-diagnostic-float-equal)
    {
        return Manifold::Empty();
    }

    return {minNormal, minDistance};
}

Manifold algo::Sat(
    const BoxCollider* colliderA, const Transform* transformA,
    const BoxCollider* colliderB, const Transform* transformB
)
{
    float overlap = std::numeric_limits<float>::max();
    core::Vec2f smallestAxis;

    const auto verticesA = colliderA->GetTransformedVertices(*transformA);
    const auto verticesB = colliderB->GetTransformedVertices(*transformB);

    const auto axesA = BoxCollider::GetAxes(verticesA);
    const auto axesB = BoxCollider::GetAxes(verticesB);

    auto findCollision = [&](const core::Vec2f axis)
    {
        const Projection pA = BoxCollider::Project(axis, verticesA);
        const Projection pB = BoxCollider::Project(axis, verticesB);

        if (!pA.Overlaps(pB))
        {
            return false;
        }

        const float o = pA.GetOverlap(pB);

        if (o < overlap)
        {
            overlap = o;
            smallestAxis = axis;
        }

        return true;
    };

    for (const auto& axis : axesA)
    {
        if (!findCollision(axis))
        {
            return Manifold::Empty();
        }
    }

    for (const auto& axis : axesB)
    {
        if (!findCollision(axis))
        {
            return Manifold::Empty();
        }
    }

    return {smallestAxis, overlap};
}
}
