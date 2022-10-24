#pragma once

#include "game_globals.hpp"
#include "engine/component.hpp"
#include "physics/event_interfaces.hpp"
#include "physics/physics_manager.hpp"

namespace game
{
struct FallingWall
{
	
};

class FallingWallManager final :
	public core::ComponentManager<FallingWall, static_cast<core::EntityMask>(ComponentType::FallingWall)>,
	public OnCollisionInterface
{
public:
	FallingWallManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, core::TransformManager& transformManager);
	void OnCollision(core::Entity entity1, core::Entity entity2) override;
	void SetFallingWall(core::Entity entity, FallingWall fallingWall);
	void SetCenter(const sf::Vector2f center) { _center = center; }
	void SetWindowSize(const sf::Vector2f newWindowSize) { _windowSize = newWindowSize; }

private:
	PhysicsManager& _physicsManager;
	core::TransformManager& _transformManager;
	sf::Vector2f _center{};
	sf::Vector2f _windowSize{};
};
}
