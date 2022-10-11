#include "game/input_manager.hpp"

#include "SFML/Window/Keyboard.hpp"

namespace game
{
PlayerInput GetPlayerInput(const int index)
{
	switch (index)
	{
	case 0:
	{
		PlayerInput clientInput1 = 0;
		clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)
			? player_input_enum::Left
			: player_input_enum::None);
		clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)
			? player_input_enum::Right
			: player_input_enum::None);
		clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)
			? player_input_enum::Up
			: player_input_enum::None);
		clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)
			? player_input_enum::Down
			: player_input_enum::None);
		clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)
			? player_input_enum::Shoot
			: player_input_enum::None);

		return clientInput1;
	}
	case 1:
	{
		PlayerInput clientInput2 = 0;
		clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::A)
			? player_input_enum::Left
			: player_input_enum::None);
		clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::D)
			? player_input_enum::Right
			: player_input_enum::None);
		clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::W)
			? player_input_enum::Up
			: player_input_enum::None);
		clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::S)
			? player_input_enum::Down
			: player_input_enum::None);
		clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)
			? player_input_enum::Shoot
			: player_input_enum::None);
		return clientInput2;
	}
	default:
		break;
	}
	return 0;
}
} // namespace game
