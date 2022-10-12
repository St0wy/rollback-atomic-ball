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

void PlayerCharacterManager::FixedUpdate(const sf::Time deltaTime)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
	{
		const auto playerEntity = _gameManager.GetEntityFromPlayerNumber(playerNumber);
		const bool isPlayer = _entityManager.HasComponent(playerEntity,
			static_cast<core::EntityMask>(ComponentType::PlayerCharacter));
		if (!isPlayer) continue;

		Rigidbody& playerBody = _physicsManager.GetRigidbody(playerEntity);
		// ReSharper disable once CppUseStructuredBinding
		PlayerCharacter& playerCharacter = GetComponent(playerEntity);
		const auto input = playerCharacter.input;

		const bool right = input & player_input_enum::PlayerInput::Right;
		const bool left = input & player_input_enum::PlayerInput::Left;
		const bool up = input & player_input_enum::PlayerInput::Up;
		const bool down = input & player_input_enum::PlayerInput::Down;

		const auto rotationOffset = ((left ? -1.0f : 0.0f) + (right ? 1.0f : 0.0f)) * PLAYER_ANGULAR_SPEED;
		const auto rotation = playerBody.Rotation() + rotationOffset;
		playerBody.SetRotation(rotation);
		auto dir = core::Vec2f::FromAngle(rotation);

		const auto acceleration = ((down ? -1.0f : 0.0f) + (up ? 1.0f : 0.0f)) * dir * PLAYER_SPEED;
		const core::Vec2f vel = acceleration * deltaTime.asSeconds();
		playerBody.ApplyForce(vel);

		if (playerCharacter.invincibilityTime > 0.0f)
		{
			playerCharacter.invincibilityTime -= deltaTime.asSeconds();
		}

		// Check if playerCharacter cannot shoot, and increase shootingTime
		if (playerCharacter.shootingTime < PLAYER_SHOOTING_PERIOD)
		{
			playerCharacter.shootingTime += deltaTime.asSeconds();
		}

		// Shooting mechanism
		if (playerCharacter.shootingTime >= PLAYER_SHOOTING_PERIOD)
		{
			if (input & player_input_enum::PlayerInput::Shoot)
			{
				const auto currentPlayerSpeed = playerBody.Velocity().GetMagnitude();
				const auto bulletVelocity = dir *
					((core::Vec2f::Dot(playerBody.Velocity(), dir) > 0.0f ? currentPlayerSpeed : 0.0f)
					+ BULLET_SPEED);
				const auto bulletPosition = playerBody.Position() + dir * 0.5f + playerBody.Position() * deltaTime.asSeconds();
				_gameManager.SpawnBullet(playerCharacter.playerNumber,
					bulletPosition,
					bulletVelocity);
				playerCharacter.shootingTime = 0.0f;
			}
		}
	}
}
}
