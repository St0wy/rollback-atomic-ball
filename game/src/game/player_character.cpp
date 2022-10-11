#include <game/game_manager.hpp>
#include <game/player_character.hpp>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
PlayerCharacterManager::PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager,
	GameManager& gameManager)
	: ComponentManager(entityManager),
	_physicsManager(physicsManager),
	_gameManager(gameManager)
{}

void PlayerCharacterManager::FixedUpdate(const sf::Time dt)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
	{
		const auto playerEntity = _gameManager.GetEntityFromPlayerNumber(playerNumber);
		if (!_entityManager.HasComponent(playerEntity,
			static_cast<core::EntityMask>(ComponentType::PlayerCharacter)))
			continue;
		Rigidbody& playerBody = _physicsManager.GetRigidbody(playerEntity);
		auto& playerCharacter = GetComponent(playerEntity);
		const auto input = playerCharacter.input;

		//const bool right = input & player_input_enum::PlayerInput::Right;
		//const bool left = input & player_input_enum::PlayerInput::Left;
		const bool up = input & player_input_enum::PlayerInput::Up;
		const bool down = input & player_input_enum::PlayerInput::Down;

		//const auto angularVelocity = ((left ? -1.0f : 0.0f) + (right ? 1.0f : 0.0f)) * PLAYER_ANGULAR_SPEED;

		//playerBody.angularVelocity = angularVelocity;

		auto dir = core::Vec2f::Up();
		//dir = dir.Rotate(-(playerBody.rotation + playerBody.angularVelocity * dt.asSeconds()));

		const auto acceleration = ((down ? -1.0f : 0.0f) + (up ? 1.0f : 0.0f)) * dir;

		const core::Vec2f vel = playerBody.Velocity() + acceleration * dt.asSeconds();
		playerBody.SetVelocity(vel);

		_physicsManager.SetBody(playerEntity, playerBody);

		if (playerCharacter.invincibilityTime > 0.0f)
		{
			playerCharacter.invincibilityTime -= dt.asSeconds();
			SetComponent(playerEntity, playerCharacter);
		}

		//Check if playerCharacter cannot shoot, and increase shootingTime
		if (playerCharacter.shootingTime < PLAYER_SHOOTING_PERIOD)
		{
			playerCharacter.shootingTime += dt.asSeconds();
			SetComponent(playerEntity, playerCharacter);
		}

		//Shooting mechanism
		if (playerCharacter.shootingTime >= PLAYER_SHOOTING_PERIOD)
		{
			if (input & player_input_enum::PlayerInput::Shoot)
			{
				const auto currentPlayerSpeed = playerBody.Velocity().GetMagnitude();
				const auto bulletVelocity = dir *
					((core::Vec2f::Dot(playerBody.Velocity(), dir) > 0.0f ? currentPlayerSpeed : 0.0f)
					+ BULLET_SPEED);
				const auto bulletPosition = playerBody.Position() + dir * 0.5f + playerBody.Position() * dt.asSeconds();
				_gameManager.SpawnBullet(playerCharacter.playerNumber,
					bulletPosition,
					bulletVelocity);
				playerCharacter.shootingTime = 0.0f;
				SetComponent(playerEntity, playerCharacter);
			}
		}
	}
}
}
