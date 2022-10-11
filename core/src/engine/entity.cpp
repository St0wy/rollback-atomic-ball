#include "engine/entity.hpp"

#include <algorithm>

#include "engine/component.hpp"

#include "utils/assert.hpp"

namespace core
{
EntityManager::EntityManager()
{
	_entityMasks.resize(ENTITY_INIT_NMB, INVALID_ENTITY_MASK);
}

EntityManager::EntityManager(const std::size_t reservedSize)
{
	_entityMasks.resize(reservedSize, INVALID_ENTITY_MASK);
}

Entity EntityManager::CreateEntity()
{
	const auto entityMaskIt = std::ranges::find_if(_entityMasks,
		[](const EntityMask entityMask)
		{
			return entityMask == INVALID_ENTITY_MASK;
		});

	if (entityMaskIt == _entityMasks.end())
	{
		const auto newEntity = _entityMasks.size();
		_entityMasks.resize(newEntity + newEntity / 2, INVALID_ENTITY_MASK);
		AddComponent(
			static_cast<Entity>(newEntity),
			static_cast<EntityMask>(ComponentType::Empty));
		return static_cast<Entity>(newEntity);
	}

	const auto newEntity = std::distance(_entityMasks.begin(), entityMaskIt);
	AddComponent(
		static_cast<Entity>(newEntity),
		static_cast<EntityMask>(ComponentType::Empty));
	return static_cast<Entity>(newEntity);
}

void EntityManager::DestroyEntity(const Entity entity)
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	_entityMasks[entity] = INVALID_ENTITY_MASK;
}

void EntityManager::AddComponent(const Entity entity, const EntityMask mask)
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	_entityMasks[entity] |= mask;
}

void EntityManager::RemoveComponent(const Entity entity, const EntityMask mask)
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	_entityMasks[entity] &= ~mask;
}

bool EntityManager::EntityExists(const Entity entity) const
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	return _entityMasks[entity] != INVALID_ENTITY_MASK;
}

std::size_t EntityManager::GetEntitiesSize() const
{
	return _entityMasks.size();
}

bool EntityManager::HasComponent(const Entity entity, const EntityMask mask) const
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	return (_entityMasks[entity] & mask) == mask;
}
}
