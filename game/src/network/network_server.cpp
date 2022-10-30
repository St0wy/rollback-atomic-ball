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
	core::LogInfo(fmt::format("[Server] Sending TCP packet: {}",
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

			if (status == sf::Socket::NotReady)
			{
				core::LogInfo(fmt::format(
					"[Server] Error trying to send packet to Player: {} socket is not ready",
					playerNumber));
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
			core::LogInfo(fmt::format("[Warning] Trying to send UDP packet, but missing port!"));
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
			//core::LogInfo("[Server] Sending UDP packet: " +
			//std::to_string(static_cast<int>(packet->packetType)));
			break;

		case sf::Socket::Disconnected:
			{
				core::LogInfo("[Server] Error while sending UDP packet, DISCONNECTED");
				break;
			}
		case sf::Socket::NotReady:
			core::LogInfo("[Server] Error while sending UDP packet, NOT READY");

			break;

		case sf::Socket::Error:
			core::LogInfo("[Server] Error while sending UDP packet, DISCONNECTED");
			break;
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

	_gameManager.GetRollbackManager().SetTextTEMP("NetworkServer");

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

	core::LogInfo(fmt::format("[Server] Tcp Socket on port: {}", _tcpPort));

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
	core::LogInfo(fmt::format("[Server] Udp Socket on port: {}", _udpPort));

	_status = _status | Open;

	_gameManager.SetupLevel();
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
			core::LogInfo(fmt::format("[Server] New player connection with address: {} and port: {}",
			                          remoteAddress.toString(),
			                          _tcpSockets[_lastSocketIndex].getRemotePort()));
			const auto connectionStatus = static_cast<std::uint8_t>(FirstPlayerConnect << _lastSocketIndex);
			_status = _status | connectionStatus;
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
				core::LogInfo(fmt::format(
					"[Error] Player Number {} is disconnected when receiving",
					playerNumber + 1));
				_status = _status & ~(FirstPlayerConnect << playerNumber);
				auto endGame = std::make_unique<LoseGamePacket>();
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

void NetworkServer::End()
{
}

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

		constexpr auto rotation = core::Degree(0);
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
			const JoinPacket joinPacket = *dynamic_cast<JoinPacket*>(packet.get());
			Server::ReceivePacket(std::move(packet));
			auto clientId = core::ConvertFromBinary<ClientId>(joinPacket.clientId);

			std::string packetTypeString = packetSource == PacketSocketSource::Udp
				                               ? fmt::format(" UDP with port: {}", port)
				                               : " TCP";
			auto unsignedId = static_cast<unsigned>(clientId);
			core::LogInfo(fmt::format("[Server] Received Join Packet from: {} {}", unsignedId, packetTypeString));

			const auto it = std::ranges::find(_clientMap, clientId);
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
				// Calculate time difference
				const auto clientTime = core::ConvertFromBinary<unsigned long>(joinPacket.startTime);
				using namespace std::chrono;
				const unsigned long deltaTime = static_cast<unsigned long>((duration_cast<milliseconds>(
					system_clock::now().time_since_epoch()).count())) - clientTime;
				core::LogInfo(fmt::format("[Server] Client Server deltaTime: {}", deltaTime));
				_clientInfoMap[playerNumber].timeDifference = deltaTime;
			}
			break;
		}
	default:
		Server::ReceivePacket(std::move(packet));
		break;
	}
}

void NetworkServer::ReceiveNetPacket(sf::Packet& packet, const PacketSocketSource packetSource,
                                     const sf::IpAddress address, const unsigned short port)
{
	auto receivedPacket = GenerateReceivedPacket(packet);

	if (receivedPacket != nullptr)
	{
		ProcessReceivePacket(std::move(receivedPacket), packetSource, address, port);
	}
}
}
