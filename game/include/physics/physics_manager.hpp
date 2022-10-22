#pragma once
#include <optional>

#include <SFML/System/Time.hpp>

#include "broad_phase_grid.hpp"
#include "collision.hpp"
#include "rigidbody.hpp"
#include "solver.hpp"
#include "event_interfaces.hpp"
#include "layers.hpp"

#include "engine/component.hpp"
#include "engine/entity.hpp"

#include "graphics/graphics.hpp"

#include "utils/action_utility.hpp"

namespace core
{
class TransformManager;
}

namespace game
{
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
	void SetRigidbody(core::Entity entity, Rigidbody& body);
	void AddRigidbody(core::Entity entity);

	void AddAabbCollider(core::Entity entity);
	void SetAabbCollider(core::Entity entity, const AabbCollider& aabbCollider);
	[[nodiscard]] AabbCollider& GetAabbCollider(core::Entity entity);

	void AddCircleCollider(core::Entity entity);
	void SetCircleCollider(core::Entity entity, const CircleCollider& circleCollider);
	[[nodiscard]] CircleCollider& GetCircleCollider(core::Entity entity);

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

	core::Vec2f _gravity = { 0, -9.81f };

	LayerCollisionMatrix _layerCollisionMatrix;

	// Used for debug
	sf::Vector2f _center{};
	sf::Vector2f _windowSize{};
};
}
