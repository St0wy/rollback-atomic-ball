#include "game/physics_manager.h"

#include <SFML/Graphics/RectangleShape.hpp>

#include "engine/transform.h"

#include "game/game_globals.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
PhysicsManager::PhysicsManager(core::EntityManager& entityManager)
	: _entityManager(entityManager), _bodyManager(entityManager), _boxManager(entityManager)
{}

constexpr bool Box2Box(const float r1X, const float r1Y, const float r1W, const float r1H, const float r2X,
	const float r2Y, const float r2W, const float r2H)
{
	return r1X + r1W >= r2X && // r1 right edge past r2 left
		r1X <= r2X + r2W && // r1 left edge past r2 right
		r1Y + r1H >= r2Y && // r1 top edge past r2 bottom
		r1Y <= r2Y + r2H;
}

void PhysicsManager::FixedUpdate(const sf::Time dt)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool isBody2D = _entityManager.HasComponent(entity, static_cast<core::EntityMask>(core::ComponentType::Body2D));
		if (!isBody2D) continue;

		auto& body = _bodyManager.GetComponent(entity);
		body.position += body.velocity * dt.asSeconds();
		body.rotation += body.angularVelocity * dt.asSeconds();
		_bodyManager.SetComponent(entity, body);
	}

	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		if (!IsActivePhysicalObject(entity)) continue;

		for (core::Entity otherEntity = entity + 1; otherEntity < _entityManager.GetEntitiesSize(); otherEntity++)
		{
			if (!_entityManager.HasComponent(otherEntity,
				static_cast<core::EntityMask>(core::ComponentType::Body2D) | static_cast<core::EntityMask>(
				core::ComponentType::BoxCollider2D)) ||
				_entityManager.HasComponent(otherEntity, static_cast<core::EntityMask>(ComponentType::Destroyed)))
				continue;
			const Body& body1 = _bodyManager.GetComponent(entity);
			const Box& box1 = _boxManager.GetComponent(entity);

			const Body& body2 = _bodyManager.GetComponent(otherEntity);
			const Box& box2 = _boxManager.GetComponent(otherEntity);

			if (Box2Box(
				body1.position.x - box1.extends.x,
				body1.position.y - box1.extends.y,
				box1.extends.x * 2.0f,
				box1.extends.y * 2.0f,
				body2.position.x - box2.extends.x,
				body2.position.y - box2.extends.y,
				box2.extends.x * 2.0f,
				box2.extends.y * 2.0f))
			{
				_onTriggerAction.Execute(entity, otherEntity);
			}
		}
	}
}

void PhysicsManager::SetBody(const core::Entity entity, const Body& body)
{
	_bodyManager.SetComponent(entity, body);
}

const Body& PhysicsManager::GetBody(const core::Entity entity) const
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

void PhysicsManager::SetBox(const core::Entity entity, const Box& box)
{
	_boxManager.SetComponent(entity, box);
}

const Box& PhysicsManager::GetBox(const core::Entity entity) const
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

		const auto& [extends, isTrigger] = _boxManager.GetComponent(entity);
		const auto& [position, velocity, angularVelocity,
			rotation, bodyType] = _bodyManager.GetComponent(entity);
		sf::RectangleShape rectShape;
		rectShape.setFillColor(core::Color::Transparent());
		rectShape.setOutlineColor(core::Color::Green());
		rectShape.setOutlineThickness(2.0f);
		rectShape.setOrigin({ extends.x * core::PIXEL_PER_METER, extends.y * core::PIXEL_PER_METER });
		rectShape.setPosition(
			position.x * core::PIXEL_PER_METER + _center.x,
			_windowSize.y - (position.y * core::PIXEL_PER_METER + _center.y));
		rectShape.setSize({ extends.x * 2.0f * core::PIXEL_PER_METER, extends.y * 2.0f * core::PIXEL_PER_METER });
		renderTarget.draw(rectShape);
	}
}

bool PhysicsManager::IsActivePhysicalObject(const core::Entity entity) const
{
	const bool hasPhysicalComponents = _entityManager.HasComponent(entity, PHYSICAL_OBJECT_MASK);
	const bool isDestroyed = _entityManager.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::Destroyed));

	return hasPhysicalComponents && !isDestroyed;
}
}
