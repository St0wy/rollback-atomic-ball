#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/UdpSocket.hpp>

#include "client.hpp"

#ifdef ENABLE_SQLITE
#include "network/debug_db.hpp"
#endif

namespace game
{
/**
 * \brief NetworkClient is a network client that uses SFML sockets.
 */
class NetworkClient final : public Client
{
public:
	enum class State
	{
		None,
		Joining,
		Joined,
		GameStarting,
		Game
	};

	enum class PacketSource
	{
		Tcp,
		Udp
	};

	void Begin() override;

	void Update(sf::Time dt) override;

	void End() override;

	void DrawImGui() override;

	void Draw(sf::RenderTarget& renderTarget) override;

	void SendReliablePacket(std::unique_ptr<Packet> packet) override;

	void SendUnreliablePacket(std::unique_ptr<Packet> packet) override;
	void SetPlayerInput(PlayerInput playerInput);

	void ReceivePacket(const Packet* packet) override;
private:
	void ReceiveNetPacket(sf::Packet& packet, PacketSource source);
	sf::UdpSocket _udpSocket;
	sf::TcpSocket _tcpSocket;

	std::string _serverAddress = "localhost";
	unsigned short _serverTcpPort = 12345;
	unsigned short _serverUdpPort = 0;


	State _currentState = State::None;

	#ifdef ENABLE_SQLITE
	DebugDatabase debugDb_;
	#endif
};
}
