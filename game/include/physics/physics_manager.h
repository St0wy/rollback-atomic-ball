#pragma once
#include <optional>

#include <SFML/System/Time.hpp>

#include "broad_phase_grid.hpp"
#include "collision.hpp"
#include "rigidbody.hpp"
#include "solver.hpp"

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
class OnTriggerInterface
{
public:
	OnTriggerInterface() = default;
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
 * \brief PhysicsManager is a class that holds both BodyManager and BoxManager and manages the physics fixed update.
 * It allows to register OnTriggerInterface to be called when a trigger occurs.
 */
class PhysicsManager final : public core::DrawInterface
{
public:
	explicit PhysicsManager(core::EntityManager& entityManager);

	static std::optional<core::ComponentType> HasCollider(const core::EntityManager& entityManager, core::Entity entity);
	[[nodiscard]] static Collider* GetCollider(
		const core::EntityManager& entityManager,
		AabbColliderManager& aabbManager,
		CircleColliderManager& circleManager,
		core::Entity entity);

	[[nodiscard]] Collider* GetCollider(core::Entity entity);

	[[nodiscard]] const Rigidbody& GetRigidbody(core::Entity entity) const;
	[[nodiscard]] Rigidbody& GetRigidbody(core::Entity entity);
	void SetBody(core::Entity entity, const Rigidbody& body);
	void AddBody(core::Entity entity);

	void AddBox(core::Entity entity);
	void SetBox(core::Entity entity, const AabbCollider& box);
	[[nodiscard]] const AabbCollider& GetBox(core::Entity entity) const;

	void SetCenter(const sf::Vector2f center) { _center = center; }
	void SetWindowSize(const sf::Vector2f newWindowSize) { _windowSize = newWindowSize; }

	void MoveBodies(sf::Time deltaTime);
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
	void SolveCollisions(const std::vector<Collision>& collisions, sf::Time deltaTime);

	//[[nodiscard]] bool IsActivePhysicalObject(core::Entity entity) const;

private:
	static void SendCollisionCallbacks(const std::vector<Collision>& collisions, core::Action<core::Entity, core::Entity>& action);

	core::EntityManager& _entityManager;
	RigidbodyManager _rigidbodyManager;

	AabbColliderManager _aabbManager;
	CircleColliderManager _circleManager;

	core::Action<core::Entity, core::Entity> _onTriggerAction;
	core::Action<core::Entity, core::Entity> _onCollisionAction;

	ImpulseSolver _impulseSolver;
	SmoothPositionSolver _smoothPositionSolver;
	BroadPhaseGrid _grid;

	//Used for debug
	sf::Vector2f _center{};
	sf::Vector2f _windowSize{};
};
}
