#pragma once

#include "game_globals.hpp"
#include "engine/component.hpp"
#include "physics/event_interfaces.hpp"
#include "physics/physics_manager.hpp"
#include "game/player_character.hpp"

namespace game
{
struct FallingObject
{
	float fallingSpeed = 1.0f;
};

class FallingObjectManager final :
	public core::ComponentManager<FallingObject, static_cast<core::EntityMask>(ComponentType::FallingObject)>
{
public:
	FallingObjectManager(core::EntityManager& entityManager, PhysicsManager& physicsManager);
	void SetFallingSpeed(core::Entity entity, float fallingSpeed);
	void FixedUpdate(sf::Time deltaTime);

private:
	PhysicsManager& _physicsManager;
};

struct FallingDoor
{
	core::Entity backgroundWallEntity = core::INVALID_ENTITY;
	bool requiresBall = false;
};

class FallingDoorManager final :
	public core::ComponentManager<FallingDoor, static_cast<core::EntityMask>(ComponentType::FallingDoor)>,
	public OnCollisionInterface
{
public:
	FallingDoorManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, 
		PlayerCharacterManager& playerCharacterManager, GameManager& gameManager);
	void SetFallingDoor(core::Entity entity, FallingDoor fallingDoor);
	void OnCollision(core::Entity entity1, core::Entity entity2) override;

private:
	void HandleCollision(core::Entity doorEntity, core::Entity playerEntity);
	PhysicsManager& _physicsManager;
	PlayerCharacterManager& _playerCharacterManager;
	GameManager& _gameManager;
};

}