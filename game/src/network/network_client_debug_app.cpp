#include "network/network_client_debug_app.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include "game/input_manager.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
void NetworkClientDebugApp::Begin()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	_windowSize = DEBUG_WINDOW_SIZE;
	for (auto& framebuffer : _clientsFramebuffers)
	{
		framebuffer.create(DEBUG_FRAMEBUFFER_SIZE.x, DEBUG_FRAMEBUFFER_SIZE.y);
	}
	for (auto& client : _clients)
	{
		client.SetWindowSize(DEBUG_FRAMEBUFFER_SIZE);
		client.Begin();
	}
}

void NetworkClientDebugApp::Update(const sf::Time dt)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	for (std::size_t i = 0; i < _clients.size(); i++)
	{
		_clients[i].SetPlayerInput(GetPlayerInput(static_cast<int>(i)));
	}

	for (auto& client : _clients)
	{
		client.Update(dt);
	}
}

void NetworkClientDebugApp::End()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	for (auto& client : _clients)
	{
		client.End();
	}
}

void NetworkClientDebugApp::DrawImGui()
{
	for (auto& client : _clients)
	{
		client.DrawImGui();
	}
}

void NetworkClientDebugApp::Draw(sf::RenderTarget& renderTarget)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB; playerNumber++)
	{
		_clientsFramebuffers[playerNumber].clear(sf::Color::Black);
		sf::RenderTarget& frameBuffer = _clientsFramebuffers[playerNumber];
		_clients[playerNumber].Draw(frameBuffer);
		_clientsFramebuffers[playerNumber].display();
	}
	_screenQuad = sf::Sprite();
	_screenQuad.setTexture(_clientsFramebuffers[0].getTexture());
	renderTarget.draw(_screenQuad);

	_screenQuad.setTexture(_clientsFramebuffers[1].getTexture());
	_screenQuad.setPosition(sf::Vector2f(static_cast<float>(_windowSize.x / 2u), 0));
	// NOLINT(bugprone-integer-division)
	renderTarget.draw(_screenQuad);
}

void NetworkClientDebugApp::OnEvent(const sf::Event&)
{
}
}
