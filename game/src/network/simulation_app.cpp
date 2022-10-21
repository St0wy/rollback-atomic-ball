#include "network/simulation_app.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include "engine/globals.hpp"

#include "game/input_manager.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
SimulationApp::SimulationApp()
	: _server(_clients)
{
	for (auto& client : _clients)
	{
		client = std::make_unique<SimulationClient>(_server);
	}
}

void SimulationApp::OnEvent(const sf::Event&)
{}

void SimulationApp::Begin()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	_windowSize = DEBUG_WINDOW_SIZE;
	for (auto& framebuffer : _clientsFramebuffers)
	{
		framebuffer.create(DEBUG_FRAMEBUFFER_SIZE.x, DEBUG_FRAMEBUFFER_SIZE.y);
	}
	for (const auto& client : _clients)
	{
		client->SetWindowSize(DEBUG_FRAMEBUFFER_SIZE);
		client->Begin();
	}
	_server.Begin();
}

void SimulationApp::Update(const sf::Time dt)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	// Read keyboard inputs
	for (std::size_t i = 0; i < _clients.size(); i++)
	{
		const PlayerInput playerInput = hasFocus ? GetPlayerInput(static_cast<int>(i)) : PlayerInput{};
		_clients[i]->SetPlayerInput(playerInput);
	}

	_server.Update(dt);
	for (const auto& client : _clients)
	{
		client->Update(dt);
	}
}

void SimulationApp::End()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	for (const auto& client : _clients)
	{
		client->End();
	}
	_server.End();
}

void SimulationApp::DrawImGui()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	_server.DrawImGui();
	for (const auto& client : _clients)
	{
		client->DrawImGui();
	}
}

void SimulationApp::Draw(sf::RenderTarget& window)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
	{
		_clientsFramebuffers[playerNumber].clear(sf::Color::Black);
		_clients[playerNumber]->Draw(_clientsFramebuffers[playerNumber]);
		_clientsFramebuffers[playerNumber].display();
	}
	_screenQuad = sf::Sprite();
	_screenQuad.setTexture(_clientsFramebuffers[0].getTexture());

	const float windowWidth = static_cast<float>(DEBUG_WINDOW_SIZE.x) / 2.0f;
	const float windowHeight = static_cast<float>(DEBUG_WINDOW_SIZE.y);
	const float framebufferWidth = static_cast<float>(DEBUG_FRAMEBUFFER_SIZE.x);
	const float framebufferHeight = static_cast<float>(DEBUG_FRAMEBUFFER_SIZE.y);

	_screenQuad.setScale({
		windowWidth / framebufferWidth,
		windowHeight / framebufferHeight
		});
	window.draw(_screenQuad);

	_screenQuad.setTexture(_clientsFramebuffers[1].getTexture());
	_screenQuad.setPosition(sf::Vector2f(static_cast<float>(_windowSize.x) / 2u, 0.0f));
	window.draw(_screenQuad);
}
}
