#include "physics/broad_phase_grid.hpp"

#include <algorithm>
#include <ranges>

#include "engine/component.h"

namespace game
{
BroadPhaseGrid::BroadPhaseGrid(
	const float minX, const float maxX,
	const float minY, const float maxY,
	const float cellSize, 
	core::EntityManager& entityManager, RigidbodyManager& rigidbodyManager
)
	: _min(minX, minY),
	_max(maxX, maxY),
	_cellSize(cellSize),
	_gridWidth(static_cast<std::size_t>(
	std::floor((_max.x - _min.x) / _cellSize))),
	_gridHeight(static_cast<std::size_t>(
	std::floor((_max.y - _min.y) / _cellSize))),
	_entityManager(entityManager), _rigidbodyManager(rigidbodyManager)
{}

void BroadPhaseGrid::Update(const std::unordered_map<std::uint64_t, core::Entity>& bodies)
{
	_grid.clear();
	_grid.resize(_gridWidth);

	for (const core::Entity entity : bodies | std::views::values)
	{
		const bool isRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));
		if (!isRigidbody) continue;

		Rigidbody& body = _rigidbodyManager.GetComponent(entity);

		const auto transform = body.Trans();
		const auto collider = bodyCol();

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

			std::vector<std::vector<Rigidbody*>>& gridCol = _grid[x];

			// Loop through each cell
			for (int y = yBodyMin; y <= yBodyMax; y++)
			{
				std::vector<Rigidbody*>& gridCell = gridCol[y];
				gridCell.push_back(body);
			}
		}
	}
}

std::vector<std::pair<std::uint64_t, std::uint64_t>> BroadPhaseGrid::GetCollisionPairs() const
{
	std::unordered_multimap<Rigidbody*, Rigidbody*> checkedCollisions;
	std::vector<std::pair<std::uint64_t, std::uint64_t>> collisions;

	for (auto& gridCol : _grid)
	{
		for (auto& gridCell : gridCol)
		{
			for (std::size_t i = 0; i < gridCell.size(); ++i)
			{
				Rigidbody* bodyA = gridCell[i];
				for (std::size_t j = i + 1; j < gridCell.size(); ++j)
				{
					Rigidbody* bodyB = gridCell[j];

					std::pair<Rigidbody*, Rigidbody*> bodyPair = bodyA < bodyB
						? std::make_pair(bodyA, bodyB)
						: std::make_pair(bodyB, bodyA);

					if (!HasBeenChecked(checkedCollisions, bodyPair))
					{
						collisions.emplace_back(bodyPair.first->id, bodyPair.second->id);
						checkedCollisions.insert(bodyPair);
					}
				}
			}
		}
	}

	return collisions;
}

bool BroadPhaseGrid::HasBeenChecked(
	const std::unordered_multimap<Rigidbody*, Rigidbody*>& checkedCollisions,
	const std::pair<Rigidbody*, Rigidbody*>& bodyPair
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
