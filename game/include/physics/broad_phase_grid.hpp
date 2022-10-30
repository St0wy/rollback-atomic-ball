#pragma once

#include <unordered_map>
#include <vector>

#include "rigidbody.hpp"

#include "engine/entity.hpp"

#include "maths/vec2.hpp"

namespace game
{
/**
* \brief A grid in which the physical object are placed in.
* This is used to optimize performance by only checking collision with colliders
* that are in the same cell.
*
* A collider that spans on multiple cells will have a pointer on every cell.
*/
class BroadPhaseGrid
{
public:
	/**
	 * \brief Constructs a new grid.
	 * \param minX X coordinate on the bottom left point of the grid.
	 * \param maxX X coordinate on the top right point of the grid.
	 * \param minY Y coordinate on the bottom left point of the grid.
	 * \param maxY Y coordinate on the top right point of the grid.
	 * \param cellSize Size (in meter) of a cell.
	 * \param entityManager Manager of the Entities.
	 * \param rigidbodyManager Manager of the Rigidbodies.
	 * \param aabbManager Manager for Aabb colliders.
	 * \param circleManager Manager for circle colliders.
	 */
	BroadPhaseGrid(float minX, float maxX, float minY, float maxY, float cellSize,
	               core::EntityManager& entityManager, RigidbodyManager& rigidbodyManager,
	               AabbColliderManager& aabbManager, CircleColliderManager& circleManager
	);

	/**
	 * \brief Updates the layout of the grid.
	 */
	void Update();

	/**
	 * \brief Find all the pair of objects that are in the same cell.
	 * Does not contain any duplicates.
	 * \return The pair of objects that will collide.
	 */
	[[nodiscard]] std::vector<std::pair<core::Entity, core::Entity>> GetCollisionPairs() const;

private:
	std::vector<std::vector<std::vector<core::Entity>>> _grid;
	core::Vec2f _min;
	core::Vec2f _max;
	float _cellSize;
	std::size_t _gridWidth;
	std::size_t _gridHeight;

	core::EntityManager& _entityManager;
	RigidbodyManager& _rigidbodyManager;
	AabbColliderManager& _aabbManager;
	CircleColliderManager& _circleManager;

	static bool HasBeenChecked(
		const std::unordered_multimap<core::Entity, core::Entity>& checkedCollisions,
		const std::pair<core::Entity, core::Entity>& bodyPair);
};
}
