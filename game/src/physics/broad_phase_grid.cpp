#include "physics/broad_phase_grid.hpp"

#include <algorithm>

#include "engine/component.h"

#include "physics/physics_manager.h"

namespace game
{
BroadPhaseGrid::BroadPhaseGrid(
	const float minX, const float maxX,
	const float minY, const float maxY,
	const float cellSize,
	core::EntityManager& entityManager, RigidbodyManager& rigidbodyManager,
	AabbColliderManager& aabbManager, CircleColliderManager& circleManager
)
	: _min(minX, minY),
	_max(maxX, maxY),
	_cellSize(cellSize),
	_gridWidth(static_cast<std::size_t>(
	std::floor((_max.x - _min.x) / _cellSize))),
	_gridHeight(static_cast<std::size_t>(
	std::floor((_max.y - _min.y) / _cellSize))),
	_entityManager(entityManager), _rigidbodyManager(rigidbodyManager),
	_aabbManager(aabbManager), _circleManager(circleManager)
{}

void BroadPhaseGrid::Update()
{
	_grid.clear();
	_grid.resize(_gridWidth);

	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool isRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));
		if (!isRigidbody) continue;

		Rigidbody& body = _rigidbodyManager.GetComponent(entity);

		const auto transform = body.Trans();

		const Collider* collider = PhysicsManager::GetCollider(_entityManager, _aabbManager, _circleManager, entity);

		if (!collider) continue;

		const core::Vec2f offsetCenter = transform->position + collider->center;

		// If body is outside the grid extents, then ignore it
		if (offsetCenter.x < _min.x || offsetCenter.x > _max.x ||
			offsetCenter.y < _min.y || offsetCenter.y > _max.y)
		{
			continue;
		}

		const core::Vec2f boundingBoxSize = collider->GetBoundingBoxSize();

		int xBodyMin = static_cast<int>(std::floor((offsetCenter.x - boundingBoxSize.x - _min.x) / _cellSize));
		xBodyMin = std::clamp(xBodyMin, 0, static_cast<int>(_gridWidth));
		int yBodyMin = static_cast<int>(std::floor((offsetCenter.y - boundingBoxSize.y - _min.y) / _cellSize));
		yBodyMin = std::clamp(yBodyMin, 0, static_cast<int>(_gridHeight));
		int xBodyMax = static_cast<int>(std::floor((offsetCenter.x + boundingBoxSize.x - _min.x) / _cellSize));
		xBodyMax = std::clamp(xBodyMax, 0, static_cast<int>(_gridWidth) - 1);
		int yBodyMax = static_cast<int>(std::floor((offsetCenter.y + boundingBoxSize.y - _min.y) / _cellSize));
		yBodyMax = std::clamp(yBodyMax, 0, static_cast<int>(_gridHeight) - 1);

		for (int x = xBodyMin; x <= xBodyMax; x++)
		{
			if (_grid[x].empty()) _grid[x].resize(_gridHeight);

			std::vector<std::vector<core::Entity>>& gridCol = _grid[x];

			// Loop through each cell
			for (int y = yBodyMin; y <= yBodyMax; y++)
			{
				std::vector<core::Entity>& gridCell = gridCol[y];
				gridCell.push_back(entity);
			}
		}
	}
}

std::vector<std::pair<core::Entity, core::Entity>> BroadPhaseGrid::GetCollisionPairs() const
{
	std::unordered_multimap<core::Entity, core::Entity> checkedCollisions;
	std::vector<std::pair<core::Entity, core::Entity>> collisions;
	collisions.reserve(64);

	for (auto& gridCol : _grid)
	{
		for (auto& gridCell : gridCol)
		{
			for (std::size_t i = 0; i < gridCell.size(); ++i)
			{
				core::Entity bodyA = gridCell[i];
				for (std::size_t j = i + 1; j < gridCell.size(); ++j)
				{
					core::Entity bodyB = gridCell[j];

					std::pair<core::Entity, core::Entity> bodyPair = bodyA < bodyB
						? std::make_pair(bodyA, bodyB)
						: std::make_pair(bodyB, bodyA);

					if (HasBeenChecked(checkedCollisions, bodyPair)) continue;

					collisions.emplace_back(bodyPair.first, bodyPair.second);
					checkedCollisions.insert(bodyPair);
				}
			}
		}
	}

	return collisions;
}

bool BroadPhaseGrid::HasBeenChecked(
	const std::unordered_multimap<core::Entity, core::Entity>& checkedCollisions,
	const std::pair<core::Entity, core::Entity>& bodyPair
)
{
	auto [first, second] =
		checkedCollisions.equal_range(bodyPair.first);
	bool isContained = false;
	for (auto& i = first; i != second; ++i)
	{
		if (i->second == bodyPair.second)
		{
			isContained = true;
			break;
		}
	}

	return isContained;
}
}
