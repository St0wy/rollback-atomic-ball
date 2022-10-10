#include "physics/physics_manager.h"

#include <SFML/Graphics/RectangleShape.hpp>

#include "engine/transform.h"

#include "game/game_globals.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
PhysicsManager::PhysicsManager(core::EntityManager& entityManager)
	: _entityManager(entityManager),
	  _bodyManager(entityManager),
	  _boxManager(entityManager),
	  _grid(-100, 100, -100, 100, 10) {}

constexpr bool Box2Box(const float r1X, const float r1Y, const float r1W, const float r1H, const float r2X,
					   const float r2Y, const float r2W, const float r2H)
{
	return r1X + r1W >= r2X && // r1 right edge past r2 left
		r1X <= r2X + r2W && // r1 left edge past r2 right
		r1Y + r1H >= r2Y && // r1 top edge past r2 bottom
		r1Y <= r2Y + r2H;
}

void PhysicsManager::FixedUpdate(const sf::Time deltaTime)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool isRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));
		if (!isRigidbody) continue;

		Rigidbody& body = _bodyManager.GetComponent(entity);
		body.Trans()->position += body.Velocity() * deltaTime.asSeconds();
		_bodyManager.SetComponent(entity, body);
	}

	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (!IsActivePhysicalObject(entity)) continue;

		for (core::Entity otherEntity = entity + 1; otherEntity < _entityManager.GetEntitiesSize(); otherEntity++)
		{
			if (!_entityManager.HasComponent(otherEntity,
					static_cast<core::EntityMask>(core::ComponentType::Rigidbody) | static_cast<core::EntityMask>(
						core::ComponentType::AabbCollider)) ||
				_entityManager.HasComponent(otherEntity, static_cast<core::EntityMask>(ComponentType::Destroyed)))
				continue;
			const Rigidbody& body1 = _bodyManager.GetComponent(entity);
			const AabbCollider& box1 = _boxManager.GetComponent(entity);

			const Rigidbody& body2 = _bodyManager.GetComponent(otherEntity);
			const AabbCollider& box2 = _boxManager.GetComponent(otherEntity);

			//if (Box2Box(
			//	body1.position.x - box1.extends.x,
			//	body1.position.y - box1.extends.y,
			//	box1.extends.x * 2.0f,
			//	box1.extends.y * 2.0f,
			//	body2.position.x - box2.extends.x,
			//	body2.position.y - box2.extends.y,
			//	box2.extends.x * 2.0f,
			//	box2.extends.y * 2.0f))
			//{
			//	_onTriggerAction.Execute(entity, otherEntity);
			//}
		}
	}
}

void PhysicsManager::SetBody(const core::Entity entity, const Rigidbody& body)
{
	_bodyManager.SetComponent(entity, body);
}

const Rigidbody& PhysicsManager::GetBody(const core::Entity entity) const
{
	return _bodyManager.GetComponent(entity);
}

void PhysicsManager::AddBody(const core::Entity entity)
{
	_bodyManager.AddComponent(entity);
}

void PhysicsManager::AddBox(const core::Entity entity)
{
	_boxManager.AddComponent(entity);
}

void PhysicsManager::SetBox(const core::Entity entity, const AabbCollider& box)
{
	_boxManager.SetComponent(entity, box);
}

const AabbCollider& PhysicsManager::GetBox(const core::Entity entity) const
{
	return _boxManager.GetComponent(entity);
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
	_bodyManager.CopyAllComponents(physicsManager._bodyManager.GetAllComponents());
	_boxManager.CopyAllComponents(physicsManager._boxManager.GetAllComponents());
}

void PhysicsManager::Draw(sf::RenderTarget& renderTarget)
{
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (!IsActivePhysicalObject(entity)) continue;

		const AabbCollider& box = _boxManager.GetComponent(entity);
		const Rigidbody& rigidbody = _bodyManager.GetComponent(entity);
		sf::RectangleShape rectShape;
		// TODO : Draw physical shape
	}
}

void PhysicsManager::ApplyGravity()
{
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool isRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));
		if (!isRigidbody) continue;

		Rigidbody& body = _bodyManager.GetComponent(entity);
		const core::Vec2f force = body.GravityForce() * body.Mass();
		body.ApplyForce(force);
		_bodyManager.SetComponent(entity, body);
	}
}

void PhysicsManager::ResolveCollisions(sf::Time deltaTime)
{
	// Vector for the collisions that have been detected
	std::vector<Collision> collisions;

	// Vector for the collisions that have been caused by trigger colliders
	std::vector<Collision> triggers;

	_grid.Update(_bodies);
}

bool PhysicsManager::IsActivePhysicalObject(const core::Entity entity) const
{
	const bool hasPhysicalComponents = _entityManager.HasComponent(entity, PHYSICAL_OBJECT_MASK);
	const bool isDestroyed = _entityManager.HasComponent(entity,
		static_cast<core::EntityMask>(ComponentType::Destroyed));

	return hasPhysicalComponents && !isDestroyed;
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
