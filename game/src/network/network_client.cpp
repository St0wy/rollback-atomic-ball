#include <chrono>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <network/network_client.h>

#include "maths/basic.h"

#include "utils/conversion.h"
#include "utils/log.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
void NetworkClient::Begin()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	_clientId = ClientId{
		core::RandomRange(std::numeric_limits<std::underlying_type_t<ClientId>>::lowest(),
			std::numeric_limits<std::underlying_type_t<ClientId>>::max())
	};
	//JOIN packet
	_gameManager.Begin();
	_tcpSocket.setBlocking(false);
	_udpSocket.setBlocking(true);
	auto status = sf::Socket::Error;
	while (status != sf::Socket::Done)
	{
		status = _udpSocket.bind(sf::Socket::AnyPort);
	}
	_udpSocket.setBlocking(false);
	#ifdef ENABLE_SQLITE
	debugDb_.Open(fmt::format("Client_{}.db", static_cast<unsigned>(clientId_)));
	#endif
}

void NetworkClient::Update(const sf::Time dt)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	Client::Update(dt);
	if (_currentState != State::None)
	{
		auto status = sf::Socket::Done;
		//Receive TCP Packet
		while (status == sf::Socket::Done)
		{
			sf::Packet packet;
			status = _tcpSocket.receive(packet);
			switch (status)
			{
			case sf::Socket::Done:
				ReceiveNetPacket(packet, PacketSource::Tcp);
				break;
			case sf::Socket::NotReady:
				//core::LogDebug("[Client] Error while receiving tcp socket is not ready");
				break;
			case sf::Socket::Partial:
				core::LogDebug("[Client] Error while receiving TCP packet, PARTIAL");
				break;
			case sf::Socket::Disconnected:
			case sf::Socket::Error:
			default:
				break;
			}
		}

		//Receive UDP packet
		status = sf::Socket::Done;
		while (status == sf::Socket::Done)
		{
			sf::Packet packet;
			sf::IpAddress sender;
			unsigned short port;
			status = _udpSocket.receive(packet, sender, port);
			switch (status)
			{
			case sf::Socket::Done:
				ReceiveNetPacket(packet, PacketSource::Udp);
				break;
			case sf::Socket::NotReady:
				break;
			case sf::Socket::Partial:
				core::LogDebug("[Client] Error while receiving UDP packet, PARTIAL");
				break;
			case sf::Socket::Disconnected:
				core::LogDebug("[Client] Error while receiving UDP packet, DISCONNECTED");
				break;
			case sf::Socket::Error:
				core::LogDebug("[Client] Error while receiving UDP packet, ERROR");
				break;
			}
		}

		switch (_currentState)
		{
		case State::Joining:
		{
			if (_serverUdpPort != 0)
			{
				//Need to send a join packet on the unreliable channel
				auto joinPacket = std::make_unique<JoinPacket>();
				joinPacket->clientId = core::ConvertToBinary<ClientId>(_clientId);
				SendUnreliablePacket(std::move(joinPacket));
			}
			break;
		}
		case State::None:
		case State::Joined:
		case State::GameStarting:
		case State::Game:
		default:
			break;
		}
	}

	_gameManager.Update(dt);
}

void NetworkClient::End()
{
	_gameManager.End();

	#ifdef ENABLE_SQLITE
	debugDb_.Close();
	#endif
}

void NetworkClient::DrawImGui()
{
	const auto windowName = "Client " + std::to_string(static_cast<unsigned>(_clientId));
	ImGui::Begin(windowName.c_str());

	if (_srtt > 0.0f)
	{
		ImGui::Text("SRTT: %f", _srtt);
		ImGui::Text("RTTVAR: %f", _rttvar);
		ImGui::Text("RTO: %f", _rto);
	}


	ImGui::InputText("Host", &_serverAddress);

	int portBuffer = _serverTcpPort;
	if (ImGui::InputInt("Port", &portBuffer))
	{
		_serverTcpPort = static_cast<unsigned short>(portBuffer);
	}
	if (_currentState == State::None &&
		ImGui::Button("Join"))
	{
		_tcpSocket.setBlocking(true);
		const auto status = _tcpSocket.connect(_serverAddress, _serverTcpPort);
		_tcpSocket.setBlocking(false);
		if (status == sf::Socket::Done)
		{
			core::LogDebug(
				"[Client] Connect to server " + _serverAddress + " with port: " + std::to_string(_serverTcpPort));
			auto joinPacket = std::make_unique<JoinPacket>();
			joinPacket->clientId = core::ConvertToBinary<ClientId>(_clientId);
			using namespace std::chrono;
			const unsigned long clientTime = static_cast<unsigned long>(duration_cast<milliseconds>(
				system_clock::now().time_since_epoch()).count());
			joinPacket->startTime = core::ConvertToBinary<unsigned long>(clientTime);
			SendReliablePacket(std::move(joinPacket));
			_currentState = State::Joining;
		}
		else
		{
			core::LogError("[Client] Error trying to connect to " + _serverAddress + " with port: " +
				std::to_string(_serverTcpPort) + " with status: " + std::to_string(status));
		}
	}
	ImGui::Text("Server UDP port: %u", _serverUdpPort);
	_gameManager.DrawImGui();
	ImGui::End();
}

void NetworkClient::Draw(sf::RenderTarget& renderTarget)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	_gameManager.Draw(renderTarget);
}

void NetworkClient::SendReliablePacket(const std::unique_ptr<Packet> packet)
{
	//core::LogDebug("[Client] Sending reliable packet to server");
	sf::Packet tcpPacket;
	GeneratePacket(tcpPacket, *packet);
	auto status = sf::Socket::Partial;
	while (status == sf::Socket::Partial)
	{
		status = _tcpSocket.send(tcpPacket);
	}
}

void NetworkClient::SendUnreliablePacket(const std::unique_ptr<Packet> packet)
{
	if (_currentState == State::None)
	{
		return;
	}
	sf::Packet udpPacket;
	GeneratePacket(udpPacket, *packet);

	switch (_udpSocket.send(udpPacket, _serverAddress, _serverUdpPort))
	{
	case sf::Socket::Done:
		//core::LogDebug("[Client] Sending UDP packet to server at host: " +
		//	serverAddress_.toString() + " port: " + std::to_string(serverUdpPort_));
		break;
	case sf::Socket::NotReady:
		core::LogDebug("[Client] Error sending UDP to server, NOT READY");
		break;
	case sf::Socket::Partial:
		core::LogDebug("[Client] Error sending UDP to server, PARTIAL");
		break;
	case sf::Socket::Disconnected:
		core::LogDebug("[Client] Error sending UDP to server, DISCONNECTED");
		break;
	case sf::Socket::Error:
		core::LogDebug("[Client] Error sending UDP to server, ERROR");
		break;
	default:
		break;
	}
}

void NetworkClient::SetPlayerInput(const PlayerInput playerInput)
{
	const auto currentFrame = _gameManager.GetCurrentFrame();
	_gameManager.SetPlayerInput(
		_gameManager.GetPlayerNumber(),
		playerInput,
		currentFrame);
}

void NetworkClient::ReceivePacket(const Packet* packet)
{
	Client::ReceivePacket(packet);
	#ifdef ENABLE_SQLITE
	switch (packet->packetType)
	{
	case PacketType::JOIN: break;
	case PacketType::SPAWN_PLAYER: break;
	case PacketType::INPUT:
	{
		auto* inputPacket = static_cast<const PlayerInputPacket*>(packet);
		debugDb_.StorePacket(inputPacket);
		break;
	}
	case PacketType::SPAWN_BULLET: break;
	case PacketType::VALIDATE_STATE:
	{
		auto* validateStatePacket = static_cast<const ValidateFramePacket*>(packet);
		const auto newValidateFrame = core::ConvertFromBinary<Frame>(validateStatePacket->newValidateFrame);
		DbPhysicsState state{};
		state.validateFrame = newValidateFrame;
		state.lastLocalValidateFrame = gameManager_.GetLastValidateFrame();
		for (size_t i = 0; i < validateStatePacket->physicsState.size(); i++)
		{
			auto* statePtr = reinterpret_cast<std::uint8_t*>(state.serverStates.data());
			statePtr[i] = validateStatePacket->physicsState[i];
		}
		for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
		{
			state.localStates[playerNumber] = gameManager_.GetRollbackManager().GetValidatePhysicsState(playerNumber);
		}
		debugDb_.StorePhysicsState(state);
		break;
	}
	case PacketType::START_GAME: break;
	case PacketType::JOIN_ACK: break;
	case PacketType::WIN_GAME: break;
	case PacketType::PING: break;
	case PacketType::NONE: break;
	default:;
}
	#endif
}

void NetworkClient::ReceiveNetPacket(sf::Packet& packet, const PacketSource source)
{
	const auto receivePacket = GenerateReceivedPacket(packet);
	Client::ReceivePacket(receivePacket.get());
	switch (receivePacket->packetType)
	{
	case PacketType::JoinAck:
	{
		core::LogDebug(
			"[Client] Receive " + std::string(source == PacketSource::Udp ? "UDP" : "TCP") + " Join ACK Packet");
		const auto* joinAckPacket = static_cast<JoinAckPacket*>(receivePacket.get());

		_serverUdpPort = core::ConvertFromBinary<unsigned short>(joinAckPacket->udpPort);
		const auto clientId = core::ConvertFromBinary<ClientId>(joinAckPacket->clientId);
		if (clientId != _clientId)
			return;
		if (source == PacketSource::Tcp)
		{
			//Need to send a join packet on the unreliable channel
			auto joinPacket = std::make_unique<JoinPacket>();
			joinPacket->clientId = core::ConvertToBinary<ClientId>(_clientId);
			SendUnreliablePacket(std::move(joinPacket));
		}
		else
		{
			if (_currentState == State::Joining)
			{
				_currentState = State::Joined;
			}
		}
		break;
	}
	case PacketType::Join:
	case PacketType::SpawnPlayer:
	case PacketType::Input:
	case PacketType::SpawnBullet:
	case PacketType::ValidateState:
	case PacketType::StartGame:
	case PacketType::WinGame:
	case PacketType::Ping:
	case PacketType::None:
	default:
		break;
	}
}
}
