#pragma once
#include <SFML/System/Time.hpp>

#include "broad_phase_grid.hpp"
#include "collision.hpp"
#include "rigidbody.hpp"

#include "engine/component.h"
#include "engine/entity.h"

#include "graphics/graphics.h"

#include "utils/action_utility.h"

namespace core
{
class TransformManager;
}

namespace game
{
/**
 * \brief OnTriggerInterface is an interface for classes that needs to be called when two boxes are in contact.
 * It needs to be registered in the PhysicsManager.
 */
class OnTriggerInterface
{
public:
	virtual ~OnTriggerInterface() = default;
	OnTriggerInterface(const OnTriggerInterface& other) = default;
	OnTriggerInterface(OnTriggerInterface&& other) = default;
	OnTriggerInterface& operator=(const OnTriggerInterface& other) = default;
	OnTriggerInterface& operator=(OnTriggerInterface&& other) = default;

	virtual void OnTrigger(core::Entity entity1, core::Entity entity2) = 0;
};

class OnCollisionInterface
{
public:
	virtual ~OnCollisionInterface() = default;
	OnCollisionInterface(const OnCollisionInterface& other) = default;
	OnCollisionInterface(OnCollisionInterface&& other) = default;
	OnCollisionInterface& operator=(const OnCollisionInterface& other) = default;
	OnCollisionInterface& operator=(OnCollisionInterface&& other) = default;

	virtual void OnCollision(core::Entity entity1, core::Entity entity2) = 0;
};

/**
 * \brief BodyManager is a ComponentManager that holds all the Body in the world.
 */
class RigidbodyManager final :
	public core::ComponentManager<Rigidbody, static_cast<core::EntityMask>(core::ComponentType::Rigidbody)>
{
public:
	using ComponentManager::ComponentManager;
};

/**
 * \brief BoxManager is a ComponentManager that holds all the Box in the world.
 */
class AabbColliderManager final :
	public core::ComponentManager<AabbCollider, static_cast<core::EntityMask>(core::ComponentType::AabbCollider)>
{
public:
	using ComponentManager::ComponentManager;
};

class CircleColliderManager final :
	public core::ComponentManager<AabbCollider, static_cast<core::EntityMask>(core::ComponentType::CircleCollider)>
{
public:
	using ComponentManager::ComponentManager;
};

/**
 * \brief PhysicsManager is a class that holds both BodyManager and BoxManager and manages the physics fixed update.
 * It allows to register OnTriggerInterface to be called when a trigger occurs.
 */
class PhysicsManager final : public core::DrawInterface
{
public:
	static constexpr core::EntityMask PHYSICAL_OBJECT_MASK =
		static_cast<core::EntityMask>(core::ComponentType::Rigidbody) |
		static_cast<core::EntityMask>(core::ComponentType::AabbCollider);

	explicit PhysicsManager(core::EntityManager& entityManager);

	[[nodiscard]] const Rigidbody& GetBody(core::Entity entity) const;
	void SetBody(core::Entity entity, const Rigidbody& body);
	void AddBody(core::Entity entity);

	void AddBox(core::Entity entity);
	void SetBox(core::Entity entity, const AabbCollider& box);
	[[nodiscard]] const AabbCollider& GetBox(core::Entity entity) const;

	void SetCenter(const sf::Vector2f center) { _center = center; }
	void SetWindowSize(const sf::Vector2f newWindowSize) { _windowSize = newWindowSize; }

	void FixedUpdate(sf::Time deltaTime);

	/**
	 * \brief RegisterTriggerListener is a method that stores an OnTriggerInterface in the PhysicsManager that will call the OnTrigger method in case of a trigger.
	 * \param onTriggerInterface is the OnTriggerInterface to be called when a trigger occurs.
	 */
	void RegisterTriggerListener(OnTriggerInterface& onTriggerInterface);
	void RegisterCollisionListener(OnCollisionInterface& onCollisionInterface);

	void CopyAllComponents(const PhysicsManager& physicsManager);
	void Draw(sf::RenderTarget& renderTarget) override;

	void ApplyGravity();
	void ResolveCollisions(sf::Time deltaTime);

	[[nodiscard]] bool IsActivePhysicalObject(core::Entity entity) const;

private:
	static void SendCollisionCallbacks(const std::vector<Collision>& collisions, core::Action<core::Entity, core::Entity>& action);

	core::EntityManager& _entityManager;
	RigidbodyManager _bodyManager;
	AabbColliderManager _boxManager;

	core::Action<core::Entity, core::Entity> _onTriggerAction;
	core::Action<core::Entity, core::Entity> _onCollisionAction;

	std::unordered_map<std::uint64_t, core::Entity> _bodies;
	BroadPhaseGrid _grid;

	//Used for debug
	sf::Vector2f _center{};
	sf::Vector2f _windowSize{};
};
}
