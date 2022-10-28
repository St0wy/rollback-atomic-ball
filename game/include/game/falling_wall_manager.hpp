#pragma once

#include "game_globals.hpp"

#include "engine/component.hpp"
#include "physics/event_interfaces.hpp"
#include "game/player_character.hpp"

namespace game
{
struct FallingWallSpawnInstructions
{
	Frame spawnFrame;
	float doorPosition;
	bool requiresBall;
};

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
	FallingDoorManager(core::EntityManager& entityManager, PlayerCharacterManager& playerCharacterManager, GameManager& gameManager);
	void SetFallingDoor(core::Entity entity, FallingDoor fallingDoor);
	void OnCollision(core::Entity entity1, core::Entity entity2) override;

private:
	void HandleCollision(core::Entity doorEntity, core::Entity playerEntity);
	PlayerCharacterManager& _playerCharacterManager;
	GameManager& _gameManager;
};

class RollbackManager;

class FallingWallSpawnManager
{
public:
	explicit FallingWallSpawnManager(RollbackManager& rollbackManager, GameManager& gameManager)
		:_rollbackManager(rollbackManager), _gameManager(gameManager)
	{}

	void FixedUpdate();
	void CopyAllComponents(const FallingWallSpawnManager& fallingWallSpawnManager);
	void SpawnWall();

	void SetNextFallingWallSpawnInstructions(const FallingWallSpawnInstructions fallingWallSpawnInstructions);

	[[nodiscard]] FallingWallSpawnInstructions GetNextFallingWallSpawnInstructions() const { return _nextFallingWallSpawnInstructions; }

private:
	FallingWallSpawnInstructions _nextFallingWallSpawnInstructions{};
	bool _hasSpawned = true;

	RollbackManager& _rollbackManager;
	GameManager& _gameManager;
};
}
