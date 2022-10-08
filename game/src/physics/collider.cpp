// ReSharper disable CppClangTidyReadabilitySuspiciousCallArgument
#include "physics/collider.hpp"

#include <array>

#include "physics/manifold.hpp"
#include "physics/manifold_factory.hpp"

namespace game
{
#pragma region BoxCollider
Manifold BoxCollider::TestCollision(const Transform* transform, const Collider* collider,
									const Transform* colliderTransform) const
{
	return collider->TestCollision(colliderTransform, this, transform);
}

Manifold BoxCollider::TestCollision(const Transform* transform, const BoxCollider* collider,
									const Transform* boxTransform) const
{
	return algo::FindBoxBoxManifold(this, transform, collider, boxTransform);
}

Manifold BoxCollider::TestCollision(
	const Transform* transform,
	const CircleCollider* collider,
	const Transform* circleTransform
) const
{
	return algo::FindBoxCircleManifold(this, transform, collider, circleTransform);
}

Manifold BoxCollider::TestCollision(const Transform*, const AabbCollider*, const Transform*) const
{
	// TODO : Implement
	return Manifold::Empty();
}

core::Vec2f BoxCollider::FindFurthestPoint(const Transform* transform, const core::Vec2f& direction) const
{
	core::Vec2f maxPoint;
	float maxDistance = -std::numeric_limits<float>::max();

	for (const core::Vec2f& vertex : GetTransformedVertices(*transform))
	{
		const float distance = core::Vec2f::Dot(vertex, direction);
		if (distance > maxDistance)
		{
			maxDistance = distance;
			maxPoint = vertex;
		}
	}

	return maxPoint;
}

std::array<core::Vec2f, 4> BoxCollider::GetVertices() const
{
	return GetTransformedVertices(
		{
			{0, 0},
			{1, 1},
			0
		}
	);
}

Projection BoxCollider::Project(const core::Vec2f& axis, const std::array<core::Vec2f, 4>& vertices)
{
	float min = core::Vec2f::Dot(axis, vertices[0]);
	float max = min;

	for (const auto& vertex : vertices)
	{
		const float p = core::Vec2f::Dot(axis, vertex);
		if (p < min)
		{
			min = p;
		}
		else if (p > max)
		{
			max = p;
		}
	}

	return {min, max};
}

std::array<core::Vec2f, 4> BoxCollider::GetAxes(const std::array<core::Vec2f, 4>& vertices)
{
	std::array<core::Vec2f, 4> axes;

	for (std::size_t i = 0; i < 4; ++i)
	{
		core::Vec2f p1 = vertices[i];
		const core::Vec2f p2 = vertices[(i + 1) % vertices.size()];
		core::Vec2f edge = p1 - p2;
		core::Vec2f normal = edge.PositivePerpendicular();
		axes[i] = normal.GetNormalized();
	}

	return axes;
}

core::Vec2f BoxCollider::GetBoundingBoxSize() const
{
	return {halfWidth * 2.0f, halfHeight * 2.0f};
}

std::array<core::Vec2f, 4> BoxCollider::GetTransformedVertices(const Transform& transform) const
{
	const float scaledHalfWidth = halfWidth * transform.scale.x;
	const float scaledHalfHeight = halfHeight * transform.scale.y;
	const core::Vec2f newCenter = center + transform.position;

	core::Vec2f topLeft = {newCenter.x - scaledHalfWidth, newCenter.y + scaledHalfHeight};
	core::Vec2f topRight = {newCenter.x + scaledHalfWidth, newCenter.y + scaledHalfHeight};
	core::Vec2f bottomRight = {newCenter.x + scaledHalfWidth, newCenter.y - scaledHalfHeight};
	core::Vec2f bottomLeft = {newCenter.x - scaledHalfWidth, newCenter.y - scaledHalfHeight};

	topLeft.RotateAround(newCenter, transform.rotation);
	topRight.RotateAround(newCenter, transform.rotation);
	bottomRight.RotateAround(newCenter, transform.rotation);
	bottomLeft.RotateAround(newCenter, transform.rotation);

	return
	{
		topLeft,
		topRight,
		bottomRight,
		bottomLeft,
	};
}
#pragma endregion

#pragma region CircleCollider

Manifold CircleCollider::TestCollision(const Transform* transform, const Collider* collider,
									   const Transform* colliderTransform) const
{
	return collider->TestCollision(colliderTransform, this, transform);
}

Manifold CircleCollider::TestCollision(const Transform* transform, const BoxCollider* collider,
									   const Transform* boxTransform) const
{
	return algo::FindCircleBoxManifold(this, transform, collider, boxTransform);
}

Manifold CircleCollider::TestCollision(const Transform* transform, const CircleCollider* collider,
									   const Transform* circleTransform) const
{
	return algo::FindCircleCircleManifold(this, transform, collider, circleTransform);
}

Manifold CircleCollider::TestCollision(const Transform* transform, const AabbCollider* collider,
									   const Transform* aabbTransform) const
{
	return algo::FindCircleAabbManifold(this, transform, collider, aabbTransform);
}

core::Vec2f CircleCollider::FindFurthestPoint(const Transform* transform, const core::Vec2f& direction) const
{
	return center + transform->position + radius * direction.GetNormalized() * transform->scale.Major();
}

core::Vec2f CircleCollider::GetBoundingBoxSize() const
{
	return {radius * 2, radius * 2};
}
#pragma endregion

#pragma region AabbCollider
Manifold AabbCollider::TestCollision(const Transform* transform, const Collider* collider,
									 const Transform* colliderTransform) const
{
	return collider->TestCollision(colliderTransform, this, transform);
}

Manifold AabbCollider::TestCollision(const Transform*, const BoxCollider*, const Transform*) const
{
	// TODO : Implement
	return Manifold::Empty();
}

Manifold AabbCollider::TestCollision(const Transform* transform, const CircleCollider* collider,
									 const Transform* circleTransform) const
{
	return algo::FindAabbCircleManifold(this, transform, collider, circleTransform);
}

Manifold AabbCollider::TestCollision(const Transform* transform, const AabbCollider* collider,
									 const Transform* aabbTransform) const
{
	return algo::FindAabbAabbManifold(this, transform, collider, aabbTransform);
}

core::Vec2f AabbCollider::FindFurthestPoint(const Transform*, const core::Vec2f&) const
{
	return {};
}

core::Vec2f AabbCollider::GetBoundingBoxSize() const
{
	return {halfWidth * 2.0f, halfHeight * 2.0f};
}
#pragma endregion
}
