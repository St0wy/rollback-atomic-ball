#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.hpp"

namespace game
{
class PhysicsManager;
class GameManager;

struct PlayerCharacter
{
	float shootingTime = 0.0f;
	PlayerInput input = 0u;
	PlayerNumber playerNumber = INVALID_PLAYER;
	short health = PLAYER_HEALTH;
	float invincibilityTime = 0.0f;
	bool hasBall = false;
	core::Radian rotation = 0.0f;
	core::Vec2f aimDirection{};
};

/**
 * \brief PlayerCharacterManager is a ComponentManager that holds all the PlayerCharacter in the game.
 */
class PlayerCharacterManager final : public core::ComponentManager<PlayerCharacter, static_cast<core::EntityMask>(ComponentType::PlayerCharacter)>
{
public:
	explicit PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager);
	void FixedUpdate(sf::Time deltaTime);

private:
	PhysicsManager& _physicsManager;
	GameManager& _gameManager;
};
}
