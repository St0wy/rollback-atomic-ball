#include "game/falling_wall_manager.hpp"

#include "engine/transform.hpp"
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

		if (!hasRigidbody || !isFallingObject) continue;

		const FallingObject& fallingObject = GetComponent(entity);
		Rigidbody& rigidbody = _physicsManager.GetRigidbody(entity);
		Transform& transform = rigidbody.Trans();
		const float deltaFall = fallingObject.fallingSpeed * deltaTime.asSeconds();
		transform.position = { transform.position.x, transform.position.y - deltaFall };

		// TODO : Destroy wall
	}
}

game::FallingDoorManager::FallingDoorManager(core::EntityManager& entityManager, PhysicsManager& physicsManager,
	PlayerCharacterManager& playerCharacterManager, GameManager& gameManager)
	: ComponentManager(entityManager),
	_physicsManager(physicsManager),
	_playerCharacterManager(playerCharacterManager),
	_gameManager(gameManager)
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
	}
}
