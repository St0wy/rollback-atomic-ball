#include "game/falling_wall_manager.hpp"

#include "engine/transform.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

game::FallingWallManager::FallingWallManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, core::TransformManager& transformManager)
	: ComponentManager(entityManager), _physicsManager(physicsManager), _transformManager(transformManager)
{}

void game::FallingWallManager::OnCollision(core::Entity, core::Entity)
{}

void game::FallingWallManager::Draw(sf::RenderTarget& renderTarget)
{
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool hasRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));
		const bool isDestroyed = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(ComponentType::Destroyed));
		const bool isFallingWall = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(ComponentType::FallingWall));
		const bool hasPosition = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Position));

		if (!hasRigidbody || isDestroyed || !isFallingWall || !hasPosition) continue;

		const FallingWall& fallingWall = GetComponent(entity);

		core::Vec2f position = _transformManager.GetPosition(entity);
		position = {
			position.x * core::PIXEL_PER_METER + _center.x,
			_windowSize.y - (position.y * core::PIXEL_PER_METER + _center.y)
		};

		const core::Vec2f fallingWallPixelSize = FALLING_WALL_SIZE * core::PIXEL_PER_METER;

		sf::RectangleShape backgroundShape(fallingWallPixelSize);
		backgroundShape.setOrigin(fallingWallPixelSize / 2.0f);
		backgroundShape.setPosition(position);
		backgroundShape.setFillColor(WALL_COLOR);

		const core::Vec2f fallingWallDoorPixelSize = FALLING_WALL_DOOR_SIZE * core::PIXEL_PER_METER;

		sf::RectangleShape doorShape(fallingWallDoorPixelSize);
		doorShape.setOrigin(fallingWallDoorPixelSize / 2.0f);
		doorShape.setPosition({ fallingWall.doorPosition, position.y });
		doorShape.setFillColor(fallingWall.requiresBall ? BALL_DOOR_COLOR : NO_BALL_DOOR_COLOR);

		renderTarget.draw(backgroundShape);
		renderTarget.draw(doorShape);
	}
}

void game::FallingWallManager::SetFallingWall(const core::Entity entity, const FallingWall fallingWall)
{
	SetComponent(entity, fallingWall);
}
