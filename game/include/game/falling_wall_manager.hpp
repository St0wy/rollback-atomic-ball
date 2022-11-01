#pragma once

#include "game_globals.hpp"
#include "score_manager.hpp"

#include "engine/component.hpp"
#include "physics/event_interfaces.hpp"
#include "game/player_character.hpp"

namespace game
{
/**
 * \brief Contains the information on how and when a falling wall should spawn.
 */
struct FallingWallSpawnInstructions
{
	Frame spawnFrame;
	float doorPosition;
	bool requiresBall;
};

/**
 * \brief Contains the information of a falling object.
 */
struct FallingObject
{
	float fallingSpeed = 1.0f;
};

/**
 * \brief Makes falling objects fall.
 */
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

/**
 * \brief Contains the information needed for a falling door on a falling wall to function.
 */
struct FallingDoor
{
	core::Entity backgroundWallEntity = core::INVALID_ENTITY;
	bool requiresBall = false;
};

/**
 * \brief Handles the behaviour of falling doors.
 */
class FallingDoorManager final :
	public core::ComponentManager<FallingDoor, static_cast<core::EntityMask>(ComponentType::FallingDoor)>,
	public OnCollisionInterface
{
public:
	FallingDoorManager(core::EntityManager& entityManager, PlayerCharacterManager& playerCharacterManager,
	                   GameManager& gameManager, ScoreManager& scoreManager);
	void SetFallingDoor(core::Entity entity, FallingDoor fallingDoor);
	void OnCollision(core::Entity entity1, core::Entity entity2) override;

private:
	void HandleCollision(core::Entity doorEntity, core::Entity playerEntity);
	PlayerCharacterManager& _playerCharacterManager;
	GameManager& _gameManager;
	ScoreManager& _scoreManager;
};

class RollbackManager;

/**
 * \brief Handles the spawn of falling walls.
 */
class FallingWallSpawnManager
{
public:
	explicit FallingWallSpawnManager(RollbackManager& rollbackManager, GameManager& gameManager)
		: _rollbackManager(rollbackManager), _gameManager(gameManager)
	{}

	void FixedUpdate();
	void CopyAllComponents(const FallingWallSpawnManager& fallingWallSpawnManager);

	/**
	 * \brief Spawns a wall using the the falling wall spawn instructions.
	 */
	void SpawnWall();

	bool SetNextFallingWallSpawnInstructions(FallingWallSpawnInstructions fallingWallSpawnInstructions);

	[[nodiscard]] FallingWallSpawnInstructions GetNextFallingWallSpawnInstructions() const
	{
		return _nextFallingWallSpawnInstructions;
	}

private:
	FallingWallSpawnInstructions _nextFallingWallSpawnInstructions{};
	bool _hasSpawned = true;

	RollbackManager& _rollbackManager;
	GameManager& _gameManager;
};
}
