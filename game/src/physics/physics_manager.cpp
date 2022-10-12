#include "physics/physics_manager.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "engine/transform.hpp"

#include "game/game_globals.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
PhysicsManager::PhysicsManager(core::EntityManager& entityManager)
	: _entityManager(entityManager),
	_rigidbodyManager(entityManager),
	_aabbManager(entityManager),
	_circleManager(entityManager), _impulseSolver(_entityManager, _rigidbodyManager),
	_smoothPositionSolver(_entityManager, _rigidbodyManager),
	_grid(-500, 500, -500, 500, 10, _entityManager, _rigidbodyManager, _aabbManager, _circleManager)
{}

std::optional<core::ComponentType> PhysicsManager::
HasCollider(const core::EntityManager& entityManager, const core::Entity entity)
{
	const bool hasAabb = entityManager.HasComponent(entity,
		static_cast<core::EntityMask>(core::ComponentType::AabbCollider));
	const bool hasCircle = entityManager.HasComponent(entity,
		static_cast<core::EntityMask>(core::ComponentType::CircleCollider));

	// It is possible that an entity has both an Aabb Collider and a circle collider
	// but I don't want to handle this case, so Aabb has more priority than the circle
	if (hasAabb)
	{
		return core::ComponentType::AabbCollider;
	}

	if (hasCircle)
	{
		return core::ComponentType::CircleCollider;
	}

	return {};
}

constexpr bool Box2Box(const float r1X, const float r1Y, const float r1W, const float r1H, const float r2X,
	const float r2Y, const float r2W, const float r2H)
{
	return r1X + r1W >= r2X && // r1 right edge past r2 left
		r1X <= r2X + r2W && // r1 left edge past r2 right
		r1Y + r1H >= r2Y && // r1 top edge past r2 bottom
		r1Y <= r2Y + r2H;
}

void PhysicsManager::MoveBodies(const sf::Time deltaTime)
{
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool hasRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));

		if (!hasRigidbody) continue;

		Rigidbody& rigidbody = GetRigidbody(entity);

		const core::Vec2f vel = rigidbody.Velocity() + rigidbody.Force() * rigidbody.InvMass() * deltaTime.asSeconds();
		rigidbody.SetVelocity(vel);

		core::Vec2f pos = rigidbody.Position() + rigidbody.Velocity() * deltaTime.asSeconds();
		rigidbody.SetPosition(pos);

		rigidbody.SetForce({ 0, 0 });
	}
}

void PhysicsManager::FixedUpdate(const sf::Time deltaTime)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	ApplyGravity();
	ResolveCollisions(deltaTime);
	MoveBodies(deltaTime);
}

void PhysicsManager::SetBody(const core::Entity entity, const Rigidbody& body)
{
	_rigidbodyManager.SetComponent(entity, body);
}

const Rigidbody& PhysicsManager::GetRigidbody(const core::Entity entity) const
{
	return _rigidbodyManager.GetComponent(entity);
}

Rigidbody& PhysicsManager::GetRigidbody(const core::Entity entity)
{
	return _rigidbodyManager.GetComponent(entity);
}

void PhysicsManager::AddBody(const core::Entity entity)
{
	_rigidbodyManager.AddComponent(entity);
}

void PhysicsManager::AddAabbCollider(const core::Entity entity)
{
	_aabbManager.AddComponent(entity);
}

void PhysicsManager::SetAabbCollider(const core::Entity entity, const AabbCollider& aabbCollider)
{
	_aabbManager.SetComponent(entity, aabbCollider);
}

AabbCollider& PhysicsManager::GetAabbCollider(const core::Entity entity)
{
	return _aabbManager.GetComponent(entity);
}

void PhysicsManager::AddCircleCollider(const core::Entity entity)
{
	_circleManager.AddComponent(entity);
}

void PhysicsManager::SetCircleCollider(const core::Entity entity, const CircleCollider& circleCollider)
{
	_circleManager.SetComponent(entity, circleCollider);
}

CircleCollider& PhysicsManager::GetCircleCollider(const core::Entity entity)
{
	return _circleManager.GetComponent(entity);
}

void PhysicsManager::RegisterTriggerListener(OnTriggerInterface& onTriggerInterface)
{
	_onTriggerAction.RegisterCallback(
		[&onTriggerInterface](const core::Entity entity1, const core::Entity entity2)
	{
		onTriggerInterface.OnTrigger(entity1, entity2);
	});
}

void PhysicsManager::RegisterCollisionListener(OnCollisionInterface& onCollisionInterface)
{
	_onCollisionAction.RegisterCallback(
		[&onCollisionInterface](const core::Entity entity1, const core::Entity entity2)
	{
		onCollisionInterface.OnCollision(entity1, entity2);
	});
}

void PhysicsManager::CopyAllComponents(const PhysicsManager& physicsManager)
{
	_rigidbodyManager.CopyAllComponents(physicsManager._rigidbodyManager.GetAllComponents());
	_aabbManager.CopyAllComponents(physicsManager._aabbManager.GetAllComponents());
}

void PhysicsManager::Draw(sf::RenderTarget& renderTarget)
{
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool hasRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));
		const bool isDestroyed = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(ComponentType::Destroyed));
		const bool hasAabbCollider = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::AabbCollider));
		const bool hasCircleCollider = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::CircleCollider));
		const bool hasCollider = hasAabbCollider || hasCircleCollider;

		if (!hasRigidbody || isDestroyed || !hasCollider) continue;

		const Rigidbody& rigidbody = _rigidbodyManager.GetComponent(entity);
		const auto& position = rigidbody.Position();

		if (hasAabbCollider)
		{
			const AabbCollider& aabbCollider = _aabbManager.GetComponent(entity);
			sf::RectangleShape rectShape;
			rectShape.setFillColor(core::Color::Transparent());
			rectShape.setOutlineColor(core::Color::Green());
			rectShape.setOutlineThickness(2.0f);

			rectShape.setOrigin({ aabbCollider.halfWidth * core::PIXEL_PER_METER, aabbCollider.halfHeight * core::PIXEL_PER_METER });
			rectShape.setPosition(
				position.x * core::PIXEL_PER_METER + _center.x,
				_windowSize.y - (position.y * core::PIXEL_PER_METER + _center.y));
			rectShape.setSize({ aabbCollider.halfWidth * 2.0f * core::PIXEL_PER_METER,
				aabbCollider.halfHeight * 2.0f * core::PIXEL_PER_METER });

			renderTarget.draw(rectShape);
		}

		if (hasCircleCollider)
		{
			const CircleCollider& circleCollider = _circleManager.GetComponent(entity);
			sf::CircleShape circleShape;
			circleShape.setFillColor(core::Color::Transparent());
			circleShape.setOutlineColor(core::Color::Green());
			circleShape.setOutlineThickness(2.0f);
			circleShape.setOrigin({ circleCollider.radius * core::PIXEL_PER_METER, circleCollider.radius * core::PIXEL_PER_METER });
			circleShape.setPosition(
				position.x * core::PIXEL_PER_METER + _center.x,
				_windowSize.y - (position.y * core::PIXEL_PER_METER + _center.y));
			circleShape.setRadius(circleCollider.radius * core::PIXEL_PER_METER);

			renderTarget.draw(circleShape);
		}
	}
}

void PhysicsManager::ApplyGravity()
{
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool hasRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));

		if (!hasRigidbody) continue;

		Rigidbody& rigidbody = GetRigidbody(entity);

		const core::Vec2f force = rigidbody.GravityForce() * rigidbody.Mass();
		rigidbody.ApplyForce(force);
		_rigidbodyManager.SetComponent(entity, rigidbody);
	}
}

void PhysicsManager::ResolveCollisions(const sf::Time deltaTime)
{
	// Vector for the collisions that have been detected
	std::vector<Collision> collisions;
	collisions.reserve(64);

	// Vector for the collisions that have been caused by trigger colliders
	std::vector<Collision> triggers;
	triggers.reserve(64);

	_grid.Update();
	const auto collisionPairs = _grid.GetCollisionPairs();

	for (auto& [firstEntity, secondEntity] : collisionPairs)
	{
		const bool firstHasRigidbody = _entityManager.HasComponent(firstEntity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));
		const bool secondHasRigidbody = _entityManager.HasComponent(secondEntity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));

		const Collider* firstCollider = GetCollider(firstEntity);
		const Collider* secondCollider = GetCollider(secondEntity);

		const bool hasColliders = firstCollider && secondCollider;
		const bool hasRigibodies = firstHasRigidbody && secondHasRigidbody;

		if (!hasColliders || !hasRigibodies) continue;

		Rigidbody& firstRigibody = GetRigidbody(firstEntity);
		Rigidbody& secondRigidbody = GetRigidbody(secondEntity);

		const Manifold manifold = firstCollider->TestCollision(firstRigibody.Trans(),
			secondCollider,
			secondRigidbody.Trans());

		if (!manifold.hasCollision) continue;

		if (firstRigibody.IsTrigger() || secondRigidbody.IsTrigger())
		{
			triggers.emplace_back(firstEntity, secondEntity, manifold);
		}
		else
		{
			collisions.emplace_back(firstEntity, secondEntity, manifold);
		}

		SolveCollisions(collisions, deltaTime);

		SendCollisionCallbacks(triggers, _onTriggerAction);
		SendCollisionCallbacks(collisions, _onCollisionAction);
	}
}

void PhysicsManager::SolveCollisions(const std::vector<Collision>& collisions, const sf::Time deltaTime)
{
	_impulseSolver.Solve(collisions, deltaTime.asSeconds());
	_smoothPositionSolver.Solve(collisions, deltaTime.asSeconds());
}

Collider* PhysicsManager::GetCollider(const core::EntityManager& entityManager, AabbColliderManager& aabbManager,
	CircleColliderManager& circleManager, const core::Entity entity)
{
	const std::optional<core::ComponentType> colliderType = HasCollider(entityManager, entity);

	if (!colliderType.has_value()) return nullptr;

	if (*colliderType == core::ComponentType::AabbCollider)
	{
		return &aabbManager.GetComponent(entity);
	}

	if (*colliderType == core::ComponentType::CircleCollider)
	{
		return &circleManager.GetComponent(entity);
	}

	return nullptr;
}

Collider* PhysicsManager::GetCollider(const core::Entity entity)
{
	return GetCollider(_entityManager, _aabbManager, _circleManager, entity);
}

void PhysicsManager::SendCollisionCallbacks(
	const std::vector<Collision>& collisions, core::Action<core::Entity, core::Entity>& action)
{
	for (const auto& [bodyA, bodyB, _] : collisions)
	{
		action.Execute(bodyA, bodyB);
	}
}
}
