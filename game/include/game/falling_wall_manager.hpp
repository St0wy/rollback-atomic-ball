#pragma once

#include "game_globals.hpp"
#include "engine/component.hpp"
#include "physics/event_interfaces.hpp"
#include "physics/physics_manager.hpp"

namespace game
{
struct FallingWall
{
	bool requiresBall;
	float doorPosition;
};

class FallingWallManager final :
	public core::ComponentManager<FallingWall, static_cast<core::EntityMask>(ComponentType::FallingWall)>,
	public OnCollisionInterface,
	public core::DrawInterface
{
public:
	FallingWallManager(core::EntityManager& entityManager, PhysicsManager& physicsManager);
	void OnCollision(core::Entity entity1, core::Entity entity2) override;
	void Draw(sf::RenderTarget& renderTarget) override;

private:
	PhysicsManager& _physicsManager;
};
}
