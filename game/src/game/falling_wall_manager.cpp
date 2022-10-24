#include "game/falling_wall_manager.hpp"

#include "engine/transform.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

game::FallingWallManager::FallingWallManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, core::TransformManager& transformManager)
	: ComponentManager(entityManager), _physicsManager(physicsManager), _transformManager(transformManager)
{}

void game::FallingWallManager::OnCollision(core::Entity, core::Entity)
{}

void game::FallingWallManager::SetFallingWall(const core::Entity entity, const FallingWall fallingWall)
{
	SetComponent(entity, fallingWall);
}
