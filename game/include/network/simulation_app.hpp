#pragma once
#include <SFML/Graphics/RenderTexture.hpp>

#include "simulation_client.hpp"
#include "simulation_server.hpp"

#include "engine/app.hpp"

#include "game/game_globals.hpp"

namespace game
{
/**
 * \brief SimulationApp is an application that owns SimulationClient and SimulationServer.
 */
class SimulationApp final : public core::App
{
public:
	SimulationApp();

	void Begin() override;

	void Update(sf::Time dt) override;

	void End() override;

	void DrawImGui() override;

	void Draw(sf::RenderTarget& window) override;

	void OnEvent(const sf::Event& event) override;
private:
	std::array<std::unique_ptr<SimulationClient>, MAX_PLAYER_NMB> _clients;
	std::array<sf::RenderTexture, MAX_PLAYER_NMB> _clientsFramebuffers;
	SimulationServer _server;
	sf::Sprite _screenQuad;
	sf::Vector2u _windowSize;
};
}
