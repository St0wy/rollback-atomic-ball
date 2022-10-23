#include "game/falling_wall_manager.hpp"

game::FallingWallManager::FallingWallManager(core::EntityManager& entityManager, PhysicsManager& physicsManager)
	: ComponentManager(entityManager), _physicsManager(physicsManager)
{}

void game::FallingWallManager::OnCollision(core::Entity entity1, core::Entity entity2)
{}

void game::FallingWallManager::Draw(sf::RenderTarget& renderTarget)
{
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool hasRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(
			core::ComponentType::Rigidbody));
		const bool isDestroyed = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(ComponentType::Destroyed));
		const bool isFallingWall = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(
			ComponentType::FallingWall));

		if (!hasRigidbody || isDestroyed || !isFallingWall) continue;

		const Rigidbody& rigidbody = _physicsManager.GetRigidbody(entity);
		const core::Vec2f position = rigidbody.Position();
	}
}
