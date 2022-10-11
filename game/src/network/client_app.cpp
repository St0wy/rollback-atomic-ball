#include "network/client_app.hpp"

#include "engine/globals.hpp"

#include "game/input_manager.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
void ClientApp::Begin()
{
	#ifdef TRACY_ENABLE
    ZoneScoped;
	#endif
	_windowSize = core::WINDOW_SIZE;
	_client.SetWindowSize(_windowSize);
	_client.Begin();
}

void ClientApp::Update(const sf::Time dt)
{
	#ifdef TRACY_ENABLE
    ZoneScoped;
	#endif
	_client.SetPlayerInput(GetPlayerInput(0));
	_client.Update(dt);
}

void ClientApp::End()
{
	#ifdef TRACY_ENABLE
    ZoneScoped;
	#endif
	_client.End();
}

void ClientApp::DrawImGui()
{
	_client.DrawImGui();
}

void ClientApp::OnEvent(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::Resized:
	{
		_windowSize = sf::Vector2u(event.size.width, event.size.height);
		_client.SetWindowSize(_windowSize);
		break;
	}
	default: ;
	}
}
} // namespace game
