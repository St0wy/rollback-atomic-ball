#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.hpp"

namespace game
{
class PhysicsManager;
class GameManager;

struct PlayerCharacter
{
	PlayerInput input = 0u;
	PlayerNumber playerNumber = INVALID_PLAYER;
	bool isDead = false;
	/**
	 * \brief Tells if the player has the ball.
	 */
	bool hasBall = false;

	/**
	 * \brief Tells if the player had the ball during the last frame, used for sprite transition.
	 */
	bool hadBall = false;
	core::Radian rotation = 0.0f;
	core::Vec2f aimDirection{};

	/**
	 * \brief Updates the state of the player to catch the ball.
	 */
	void CatchBall();

	/**
	 * \brief Updates the state of the player to throw the ball.
	 */
	void ThrowBall();
};

/**
 * \brief PlayerCharacterManager is a ComponentManager that holds all the PlayerCharacter in the game.
 */
class PlayerCharacterManager final : public core::ComponentManager<
		PlayerCharacter, static_cast<core::EntityMask>(ComponentType::PlayerCharacter)>
{
public:
	PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager,
	                       GameManager& gameManager);
	void FixedUpdate(sf::Time deltaTime);
	void AddComponent(core::Entity entity) override;

private:
	PhysicsManager& _physicsManager;
	GameManager& _gameManager;
};
}
