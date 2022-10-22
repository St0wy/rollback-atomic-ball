#pragma once

#include <cstdint>

#include "engine/entity.hpp"
#include "engine/globals.hpp"

#include "utils/assert.hpp"


namespace core
{
using Component = std::uint32_t;

enum class ComponentType : Component
{
	Empty = 1u,
	Position = 1u << 1u,
	Scale = 1u << 2u,
	Rotation = 1u << 3u,
	Transform = Position | Scale | Rotation,
	Sprite = 1u << 4u,
	RectangleShape = 1u << 5u,
	Rigidbody = 1u << 6u,
	AabbCollider = 1u << 7u,
	CircleCollider = 1u << 8u,
	OtherType = 1u << 9u,
};

/**
 * \brief ComponentManager is a class that owns Component in a contiguous array. Component indexing is done with an Entity.
 * \tparam T type of the component
 * \tparam C unique binary flag of the component. This will be set in the EntityMask of the EntityManager when added.
 */
template <typename T, Component C>
class ComponentManager
{
public:
	explicit ComponentManager(EntityManager& entityManager)
		: _entityManager(entityManager)
	{
		_components.resize(ENTITY_INIT_NMB);
	}

	virtual ~ComponentManager() = default;

	ComponentManager(const ComponentManager&) = delete;
	ComponentManager& operator=(ComponentManager&) = delete;
	ComponentManager(ComponentManager&&) = delete;
	ComponentManager& operator=(ComponentManager&&) = delete;

	/**
	 * \brief AddComponent is a method that sets the flag C in the EntityManager and resize if need the components_ array.
	 * \param entity will have its flag C added in EntityManager
	 */
	virtual void AddComponent(Entity entity);
	/**
	 * \brief RemoveComponent is a method that unsets the flag C in the EntityManager
	 * \param entity will have its flag C removed
	 */
	virtual void RemoveComponent(Entity entity);
	/**
	 * \brief GetComponent is a method that gets a constant reference to a Component given the Entity
	 * \param entity is the one that we want the Component of.
	 * \return the constant reference to the Component of Entity entity.
	 */
	[[nodiscard]] const T& GetComponent(Entity entity) const;
	/**
	 * \brief GetComponent is a method that gets a reference to a Component given the Entity
	 * \param entity is the one that we want the Component of.
	 * \return the reference to the Component of Entity entity.
	 */
	[[nodiscard]] T& GetComponent(Entity entity);
	/**
	 * \brief SetComponent is a method that sets a new value of the Component of an Entity.
	 * \param entity will have its component set.
	 * \param value is the new value that will be set.
	 */
	void SetComponent(Entity entity, const T& value);
	/**
	 * \brief GetAllComponents is a method that returns the internal array of components
	 * \return the internal array of components
	 */
	[[nodiscard]] const std::vector<T>& GetAllComponents() const;
	/**
	 * \brief CopyAllComponents is a method that changes the internal components array by copying a newly provided one.
	 * It is used by the RollbackManager when reverting the current game world data with the last validated game world data.
	 * \param components is the new component array to be copy instead of the old components array
	 */
	void CopyAllComponents(const std::vector<T>& components);
protected:
	EntityManager& _entityManager;
	std::vector<T> _components;
};

template <typename T, Component C>
void ComponentManager<T, C>::AddComponent(Entity entity)
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	//Invalid entity would allocate too much memory
	if (entity == INVALID_ENTITY)
		return;
	// Resize components array if too small
	auto newSize = _components.size();
	if (newSize == 0)
	{
		newSize = 2;
	}
	while (entity >= newSize)
	{
		newSize = newSize + newSize / 2;
	}
	_components.resize(newSize);

	_entityManager.AddComponent(entity, C);
}

template <typename T, Component C>
void ComponentManager<T, C>::RemoveComponent(const Entity entity)
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	gpr_warn(_entityManager.HasComponent(entity, C), "Entity has not the removing component");
	_entityManager.RemoveComponent(entity, C);
}

template <typename T, Component C>
const T& ComponentManager<T, C>::GetComponent(Entity entity) const
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	gpr_warn(_entityManager.HasComponent(entity, C), "Entity has not the requested component");
	return _components[entity];
}

template <typename T, Component C>
T& ComponentManager<T, C>::GetComponent(Entity entity)
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	gpr_warn(_entityManager.HasComponent(entity, C), "Entity has not the requested component");
	return _components[entity];
}

template <typename T, Component C>
void ComponentManager<T, C>::SetComponent(Entity entity, const T& value)
{
	gpr_assert(entity != INVALID_ENTITY, "Invalid Entity");
	gpr_warn(_entityManager.HasComponent(entity, C), "Entity has not the requested component");
	_components[entity] = value;
}

template <typename T, Component C>
const std::vector<T>& ComponentManager<T, C>::GetAllComponents() const
{
	return _components;
}

template <typename T, Component C>
void ComponentManager<T, C>::CopyAllComponents(const std::vector<T>& components)
{
	_components = components;
}
} // namespace core
