#pragma once
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Window/Event.hpp>

#include "client.hpp"
#include "network_client.hpp"

#include "engine/app.hpp"

#include "game/game_globals.hpp"


namespace game
{
/**
 * \brief NetworkClientDebugApp is a class that have several NetworkClient and shows them on the screen, each in their own RenderTexture.
 * It allows to debug network apps as easily as the SimulationApp.
 */
class NetworkClientDebugApp final : public core::App
{
public:
	void Begin() override;

	void Update(sf::Time dt) override;

	void End() override;

	void DrawImGui() override;

	void Draw(sf::RenderTarget& renderTarget) override;

	void OnEvent(const sf::Event& event) override;
private:
	std::array<NetworkClient, MAX_PLAYER_NMB> _clients;
	std::array<sf::RenderTexture, MAX_PLAYER_NMB> _clientsFramebuffers;
	sf::Sprite _screenQuad;
	sf::Vector2u _windowSize;
};
}
