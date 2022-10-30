#pragma once

#include "collider.hpp"
#include "manifold.hpp"

#include "transform.hpp"

namespace game
{
struct Rigidbody;

/**
 * \brief Namespace containing all the methods to get manifolds from collisions.
 * This is here to separate the logic from the Collider class.
 * Avoids the problem where we don't know if the circle-box collision resolution should be
 * in the CircleCollider class or the box ColliderClass.
 */
namespace algo
{
/**
 * \brief Finds the collision manifold between A and B.
 * \param a Circle collider of the object A.
 * \param ta Transform of the object A.
 * \param b Circle collider of the object B.
 * \param tb Transform of the object B.
 * \return The manifold of the collisions between A and B.
 */
Manifold FindCircleCircleManifold(
	const CircleCollider* a, const Transform* ta,
	const CircleCollider* b, const Transform* tb);

/**
 * \brief Finds the collision manifold between A and B.
 * \param a AABB collider of the object A.
 * \param ta Transform of the object A.
 * \param b AABB collider of the object B.
 * \param tb Transform of the object B.
 * \return The manifold of the collisions between A and B.
 */
Manifold FindAabbAabbManifold(
	const AabbCollider* a, const Transform* ta,
	const AabbCollider* b, const Transform* tb);

/**
 * \brief Finds the collision manifold between A and B.
 * \param a AABB collider of the object A.
 * \param ta Transform of the object A.
 * \param b Circle collider of the object B.
 * \param tb Transform of the object B.
 * \return The manifold of the collisions between A and B.
 */
Manifold FindAabbCircleManifold(
	const AabbCollider* a, const Transform* ta,
	const CircleCollider* b, const Transform* tb);

/**
 * \brief Finds the collision manifold between A and B.
 * \param a Circle collider of the object A.
 * \param ta Transform of the object A.
 * \param b AABB collider of the object B.
 * \param tb Transform of the object B.
 * \return The manifold of the collisions between A and B.
 */
Manifold FindCircleAabbManifold(
	const CircleCollider* a, const Transform* ta,
	const AabbCollider* b, const Transform* tb);
}
}
