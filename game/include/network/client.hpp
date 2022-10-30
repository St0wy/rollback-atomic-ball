#pragma once
#include "packet_type.hpp"

#include "game/game_manager.hpp"

#include "graphics/graphics.hpp"

namespace game
{
/**
 * \brief Client is an interface of a player game manager and the net client interface (receive and send packets).
 * A client needs an ID which is receive by the server through a packet.
 */
class Client : public core::DrawInterface, public core::DrawImGuiInterface, public PacketSenderInterface,
               public core::SystemInterface
{
public:
	Client()
		: _gameManager(*this)
	{
	}

	virtual void SetWindowSize(const sf::Vector2u windowSize)
	{
		_gameManager.SetWindowSize(windowSize);
	}

	/**
	 * \brief ReceiveNetPacket is a method called by an app owning a client when receiving a packet.
	 * It is the same one for simulated and network client
	 * \param packet A non-owning pointer to a packet (you don't need to care about deleting it
	 */
	virtual void ReceivePacket(const Packet* packet);

	void Update(sf::Time dt) override;
protected:
	ClientGameManager _gameManager;
	ClientId _clientId = INVALID_CLIENT_ID;
	float _pingTimer = -1.0f;
	float _currentPing = 0.0f;
	static constexpr float PING_PERIOD_ = 0.3f;

	float _srtt = -1.0f;
	float _rttvar = 0.0f;
	float _rto = 1.0f;
	static constexpr float K = 4.0f;
	static constexpr float G = 100.0f;
	static constexpr float ALPHA = 1.0f / 8.0f;
	static constexpr float BETA = 1.0f / 4.0f;
};
}
