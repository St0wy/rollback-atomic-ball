// ReSharper disable CppClangTidyReadabilitySuspiciousCallArgument
#include "physics/collider.hpp"

#include <array>

#include "physics/manifold.hpp"
#include "physics/manifold_factory.hpp"

namespace game
{
#pragma region CircleCollider

Manifold CircleCollider::TestCollision(const Transform* transform, const Collider* collider,
									   const Transform* colliderTransform) const
{
	return collider->TestCollision(colliderTransform, this, transform);
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
