#pragma once
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpListener.hpp>

#include "network_client.hpp"
#include "server.hpp"

#include "game/game_globals.hpp"

namespace game
{
/**
 * \brief ClientInfo is a struct used by a network server to store all needed infos about a client
 */
struct ClientInfo
{
	ClientId clientId = INVALID_CLIENT_ID;
	unsigned long long timeDifference = 0;
	sf::IpAddress udpRemoteAddress;
	unsigned short udpRemotePort = 0;
};

/**
 * \brief NetworkServer is a network server using SFML sockets.
 */
class NetworkServer final : public Server
{
public:
	enum class PacketSocketSource
	{
		Tcp,
		Udp
	};

	void SendReliablePacket(std::unique_ptr<Packet> packet) override;

	void SendUnreliablePacket(std::unique_ptr<Packet> packet) override;

	void Begin() override;

	void Update(sf::Time dt) override;

	void End() override;

	void SetTcpPort(unsigned short i);

	[[nodiscard]] bool IsOpen() const;

protected:
	void SpawnNewPlayer(ClientId clientId, PlayerNumber newPlayerNumber) override;

private:
	void ProcessReceivePacket(std::unique_ptr<Packet> packet,
		PacketSocketSource packetSource,
		sf::IpAddress address = "localhost",
		unsigned short port = 0);

	void ReceiveNetPacket(sf::Packet& packet, PacketSocketSource packetSource,
		sf::IpAddress address = "localhost",
		unsigned short port = 0);

	enum ServerStatus
	{
		Open = 1u << 0u,
		Started = 1u << 1u,
		FirstPlayerConnect = 1u << 2u,
	};

	sf::UdpSocket _udpSocket;
	sf::TcpListener _tcpListener;
	std::array<sf::TcpSocket, MAX_PLAYER_NMB> _tcpSockets;

	std::array<ClientInfo, MAX_PLAYER_NMB> _clientInfoMap{};


	unsigned short _tcpPort = 12345;
	unsigned short _udpPort = 12345;
	std::uint32_t _lastSocketIndex = 0;
	std::uint8_t _status = 0;

	#ifdef ENABLE_SQLITE
	DebugDatabase db_;
	#endif
};
}
