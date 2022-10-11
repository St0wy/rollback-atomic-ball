#include <chrono>

#include <fmt/format.h>

#include <network/network_server.hpp>

#include "utils/assert.hpp"
#include "utils/conversion.hpp"
#include "utils/log.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
void NetworkServer::SendReliablePacket(const std::unique_ptr<Packet> packet)
{
	core::LogDebug(fmt::format("[Server] Sending TCP packet: {}",
		std::to_string(static_cast<int>(packet->packetType))));
	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB;
		 playerNumber++)
	{
		sf::Packet sendingPacket;
		GeneratePacket(sendingPacket, *packet);

		auto status = sf::Socket::Partial;
		while (status == sf::Socket::Partial)
		{
			status = _tcpSockets[playerNumber].send(sendingPacket);
			switch (status)
			{
			case sf::Socket::NotReady:
				core::LogDebug(fmt::format(
					"[Server] Error trying to send packet to Player: {} socket is not ready",
					playerNumber));
				break;
			case sf::Socket::Disconnected:
				break;
			default:
				break;
			}
		}
	}
}

void NetworkServer::SendUnreliablePacket(const std::unique_ptr<Packet> packet)
{
	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB;
		 playerNumber++)
	{
		if (_clientInfoMap[playerNumber].udpRemotePort == 0)
		{
			core::LogDebug(fmt::format("[Warning] Trying to send UDP packet, but missing port!"));
			continue;
		}

		sf::Packet sendingPacket;
		GeneratePacket(sendingPacket, *packet);
		// ReSharper disable once CppTooWideScope
		const auto status = _udpSocket.send(sendingPacket,
			_clientInfoMap[playerNumber].udpRemoteAddress,
			_clientInfoMap[playerNumber].udpRemotePort);
		switch (status)
		{
		case sf::Socket::Done:
			//core::LogDebug("[Server] Sending UDP packet: " +
			//std::to_string(static_cast<int>(packet->packetType)));
			break;

		case sf::Socket::Disconnected:
		{
			core::LogDebug("[Server] Error while sending UDP packet, DISCONNECTED");
			break;
		}
		case sf::Socket::NotReady:
			core::LogDebug("[Server] Error while sending UDP packet, NOT READY");

			break;

		case sf::Socket::Error:
			core::LogDebug("[Server] Error while sending UDP packet, DISCONNECTED");
			break;
		case sf::Socket::Partial:
		default:
			break;
		}
	}
}

void NetworkServer::Begin()
{
	#ifdef TRACY_ENABLE
    ZoneScoped;
	#endif
	sf::Socket::Status status = sf::Socket::Error;
	while (status != sf::Socket::Done)
	{
		// bind the listener to a port
		status = _tcpListener.listen(_tcpPort);
		if (status != sf::Socket::Done)
		{
			_tcpPort++;
		}
	}

	_tcpListener.setBlocking(false);
	for (auto& socket : _tcpSockets)
	{
		socket.setBlocking(false);
	}

	core::LogDebug(fmt::format("[Server] Tcp Socket on port: {}", _tcpPort));

	status = sf::Socket::Error;
	while (status != sf::Socket::Done)
	{
		status = _udpSocket.bind(_udpPort);
		if (status != sf::Socket::Done)
		{
			_udpPort++;
		}
	}

	_udpSocket.setBlocking(false);
	core::LogDebug(fmt::format("[Server] Udp Socket on port: {}", _udpPort));

	_status = _status | Open;
}

void NetworkServer::Update([[maybe_unused]] sf::Time dt)
{
	#ifdef TRACY_ENABLE
    ZoneScoped;
	#endif
	if (_lastSocketIndex < MAX_PLAYER_NMB)
	{
		const sf::Socket::Status status = _tcpListener.accept(
			_tcpSockets[_lastSocketIndex]);
		if (status == sf::Socket::Done)
		{
			const auto remoteAddress = _tcpSockets[_lastSocketIndex].
				getRemoteAddress();
			core::LogDebug(fmt::format("[Server] New player connection with address: {} and port: {}",
				remoteAddress.toString(),
				_tcpSockets[_lastSocketIndex].getRemotePort()));
			_status = _status | (FirstPlayerConnect << _lastSocketIndex);
			_lastSocketIndex++;
		}
	}

	for (PlayerNumber playerNumber = 0; playerNumber < MAX_PLAYER_NMB;
		 playerNumber++)
	{
		sf::Packet tcpPacket;
		switch (_tcpSockets[playerNumber].receive(tcpPacket))
		{
		case sf::Socket::Done:
			ReceiveNetPacket(tcpPacket, PacketSocketSource::Tcp);
			break;
		case sf::Socket::Disconnected:
		{
			core::LogDebug(fmt::format(
				"[Error] Player Number {} is disconnected when receiving",
				playerNumber + 1));
			_status = _status & ~(FirstPlayerConnect << playerNumber);
			auto endGame = std::make_unique<WinGamePacket>();
			SendReliablePacket(std::move(endGame));
			_status = _status & ~Open; //Close the server
			break;
		}
		default:
			break;
		}
	}
	sf::Packet udpPacket;
	sf::IpAddress address;
	unsigned short port;
	const auto status = _udpSocket.receive(udpPacket, address, port);
	if (status == sf::Socket::Done)
	{
		ReceiveNetPacket(udpPacket, PacketSocketSource::Udp, address, port);
	}
}

void NetworkServer::End() {}

void NetworkServer::SetTcpPort(const unsigned short i)
{
	_tcpPort = i;
}

bool NetworkServer::IsOpen() const
{
	return _status & Open;
}


void NetworkServer::SpawnNewPlayer(
	[[maybe_unused]] ClientId clientId, [[maybe_unused]] PlayerNumber newPlayerNumber)
{
	//Spawning the new player in the arena
	for (PlayerNumber p = 0; p <= _lastPlayerNumber; p++)
	{
		auto spawnPlayer = std::make_unique<SpawnPlayerPacket>();
		spawnPlayer->clientId = core::ConvertToBinary(_clientMap[p]);
		spawnPlayer->playerNumber = p;

		const auto pos = SPAWN_POSITIONS[p] * 3.0f;
		spawnPlayer->pos = ConvertToBinary(pos);

		const auto rotation = SPAWN_ROTATIONS[p];
		spawnPlayer->angle = ConvertToBinary(rotation);
		_gameManager.SpawnPlayer(p, pos, rotation);

		SendReliablePacket(std::move(spawnPlayer));
	}
}


void NetworkServer::ProcessReceivePacket(
	std::unique_ptr<Packet> packet,
	const PacketSocketSource packetSource,
	const sf::IpAddress address,
	unsigned short port)
{
	switch (packet->packetType)
	{
	case PacketType::Join:
	{
		const auto joinPacket = *static_cast<JoinPacket*>(packet.get());
		Server::ReceivePacket(std::move(packet));
		auto clientId = core::ConvertFromBinary<ClientId>(joinPacket.clientId);
		core::LogDebug(fmt::format("[Server] Received Join Packet from: {} {}",
			static_cast<unsigned>(clientId),
			(packetSource == PacketSocketSource::Udp ? fmt::format(" UDP with port: {}", port) : " TCP")));
		const auto it = std::find(_clientMap.begin(), _clientMap.end(), clientId);
		PlayerNumber playerNumber = 0;
		if (it != _clientMap.end())
		{
			playerNumber = static_cast<PlayerNumber>(std::distance(_clientMap.begin(), it));
			_clientInfoMap[playerNumber].clientId = clientId;
		}
		else
		{
			gpr_assert(false, "Player Number is supposed to be already set before join!");
		}

		auto joinAckPacket = std::make_unique<JoinAckPacket>();
		joinAckPacket->clientId = core::ConvertToBinary(clientId);
		joinAckPacket->clientId = core::ConvertToBinary(clientId);
		joinAckPacket->udpPort = core::ConvertToBinary(_udpPort);
		if (packetSource == PacketSocketSource::Udp)
		{
			auto& clientInfo = _clientInfoMap[playerNumber];
			clientInfo.udpRemoteAddress = address;
			clientInfo.udpRemotePort = port;
			SendUnreliablePacket(std::move(joinAckPacket));
		}
		else
		{
			SendReliablePacket(std::move(joinAckPacket));
			//Calculate time difference
			const auto clientTime = core::ConvertFromBinary<unsigned long>(joinPacket.startTime);
			using namespace std::chrono;
			const unsigned long deltaTime = static_cast<unsigned long>((duration_cast<milliseconds>(
				system_clock::now().time_since_epoch()).count())) - clientTime;
			core::LogDebug(fmt::format("[Server] Client Server deltaTime: {}", deltaTime));
			_clientInfoMap[playerNumber].timeDifference = deltaTime;
		}
		break;
	}
	default:
		Server::ReceivePacket(std::move(packet));
		break;
	case PacketType::SpawnPlayer:
		break;
	case PacketType::Input:
		break;
	case PacketType::SpawnBullet:
		break;
	case PacketType::ValidateState:
		break;
	case PacketType::StartGame:
		break;
	case PacketType::JoinAck:
		break;
	case PacketType::WinGame:
		break;
	case PacketType::Ping:
		break;
	case PacketType::None:
		break;
	}
}

void NetworkServer::ReceiveNetPacket(sf::Packet& packet,
									 PacketSocketSource packetSource,
									 sf::IpAddress address,
									 unsigned short port)
{
	auto receivedPacket = GenerateReceivedPacket(packet);

	if (receivedPacket != nullptr)
	{
		ProcessReceivePacket(std::move(receivedPacket), packetSource, address, port);
	}
}
}
