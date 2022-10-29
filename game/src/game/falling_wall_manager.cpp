#include "game/falling_wall_manager.hpp"
#include "game/rollback_manager.hpp"
#include "game/game_manager.hpp"


game::FallingObjectManager::FallingObjectManager(core::EntityManager& entityManager, PhysicsManager& physicsManager)
	: ComponentManager(entityManager), _physicsManager(physicsManager)
{}

void game::FallingObjectManager::SetFallingSpeed(const core::Entity entity, const float fallingSpeed)
{
	_components[entity].fallingSpeed = fallingSpeed;
}

void game::FallingObjectManager::FixedUpdate(const sf::Time deltaTime)
{
	for (core::Entity entity = 0; entity < _entityManager.GetEntitiesSize(); entity++)
	{
		const bool hasRigidbody = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(core::ComponentType::Rigidbody));
		const bool isFallingObject = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(ComponentType::FallingObject));
		const bool isDestroyed = _entityManager.HasComponent(entity,
			static_cast<core::EntityMask>(ComponentType::Destroyed));

		if (!hasRigidbody || !isFallingObject || isDestroyed) continue;

		const FallingObject& fallingObject = GetComponent(entity);
		Rigidbody& rigidbody = _physicsManager.GetRigidbody(entity);
		Transform& transform = rigidbody.Trans();
		const float deltaFall = fallingObject.fallingSpeed * deltaTime.asSeconds();
		transform.position = { transform.position.x, transform.position.y - deltaFall };
	}
}

game::FallingDoorManager::FallingDoorManager(core::EntityManager& entityManager, PlayerCharacterManager& playerCharacterManager, GameManager& gameManager, ScoreManager& scoreManager)
	: ComponentManager(entityManager),
	_playerCharacterManager(playerCharacterManager),
	_gameManager(gameManager),
	_scoreManager(scoreManager)
{}

void game::FallingDoorManager::SetFallingDoor(const core::Entity entity, const FallingDoor fallingDoor)
{
	SetComponent(entity, fallingDoor);
}

void game::FallingDoorManager::OnCollision(const core::Entity entity1, const core::Entity entity2)
{
	const bool oneIsDoor = _entityManager.HasComponent(entity1,
		static_cast<core::EntityMask>(ComponentType::FallingDoor));
	const bool twoIsPlayer = _entityManager.HasComponent(entity2,
		static_cast<core::EntityMask>(ComponentType::PlayerCharacter));

	const bool oneIsPlayer = _entityManager.HasComponent(entity1,
		static_cast<core::EntityMask>(ComponentType::PlayerCharacter));
	const bool twoIsDoor = _entityManager.HasComponent(entity2,
		static_cast<core::EntityMask>(ComponentType::FallingDoor));

	if (oneIsDoor && twoIsPlayer)
	{
		HandleCollision(entity1, entity2);
	}
	else if (oneIsPlayer && twoIsDoor)
	{
		HandleCollision(entity2, entity1);
	}
}

void game::FallingDoorManager::HandleCollision(const core::Entity doorEntity, const core::Entity playerEntity)
{
	const FallingDoor& door = GetComponent(doorEntity);
	const PlayerCharacter& player = _playerCharacterManager.GetComponent(playerEntity);

	const bool requireAndHasBall = door.requiresBall && player.hasBall;
	const bool notRequireAndDoesNotHaveBall = !door.requiresBall && !player.hasBall;

	if (requireAndHasBall || notRequireAndDoesNotHaveBall)
	{
		_gameManager.DestroyEntity(door.backgroundWallEntity);
		_gameManager.DestroyEntity(doorEntity);
		_scoreManager.AddScore(DESTROY_WALL_SCORE_INCREMENT);
	}
}

void game::FallingWallSpawnManager::FixedUpdate()
{
	if (_nextFallingWallSpawnInstructions.spawnFrame == 0u) return;
	if (_hasSpawned) return;

	if (_nextFallingWallSpawnInstructions.spawnFrame <= _rollbackManager.GetLastValidateFrame())
	{
		SpawnWall();
	}
}

void game::FallingWallSpawnManager::CopyAllComponents(const FallingWallSpawnManager& fallingWallSpawnManager)
{
	_nextFallingWallSpawnInstructions = fallingWallSpawnManager._nextFallingWallSpawnInstructions;
	_hasSpawned = fallingWallSpawnManager._hasSpawned;
}

void game::FallingWallSpawnManager::SpawnWall()
{
	_hasSpawned = true;
	core::LogInfo(fmt::format("[{}] Spawning wall on frame {}", name, _gameManager.GetLastValidateFrame()));

	_gameManager.SpawnFallingWall(_nextFallingWallSpawnInstructions.doorPosition, _nextFallingWallSpawnInstructions.requiresBall);
}

bool game::FallingWallSpawnManager::SetNextFallingWallSpawnInstructions(
	const FallingWallSpawnInstructions fallingWallSpawnInstructions)
{
	if (!_hasSpawned)
	{
		core::LogWarning(fmt::format("[{}] Tried to set spawning wall instructions when the wall hasn't spawned yet", name));
		return false;
	}

	_nextFallingWallSpawnInstructions = fallingWallSpawnInstructions;
	_hasSpawned = false;
	core::LogInfo(fmt::format("[{}] I will spawn on frame : {}", name, _nextFallingWallSpawnInstructions.spawnFrame));
	return true;
}
