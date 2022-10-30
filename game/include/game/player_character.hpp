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
	bool hasBall = false;
	bool hadBall = false;
	core::Radian rotation = 0.0f;
	core::Vec2f aimDirection{};

	void CatchBall();
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
