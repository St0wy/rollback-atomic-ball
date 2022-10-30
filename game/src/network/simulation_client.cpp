#include <imgui.h>

#include <maths/basic.hpp>

#include <network/simulation_client.hpp>
#include <network/simulation_server.hpp>

#include "utils/conversion.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
SimulationClient::SimulationClient(SimulationServer& server)
	: _server(server)
{
	_gameManager.GetRollbackManager().SetTextTEMP("SimulationClient");
}

void SimulationClient::Begin()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	_clientId = ClientId{
		core::RandomRange(std::numeric_limits<std::underlying_type_t<ClientId>>::lowest(),
		                  std::numeric_limits<std::underlying_type_t<ClientId>>::max())
	};
	#ifdef ENABLE_SQLITE
	debugDb_.Open(fmt::format("Client_{}.db", static_cast<unsigned>(clientId_)));
	#endif
	//JOIN packet
	_gameManager.Begin();
}

void SimulationClient::Update(const sf::Time dt)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	Client::Update(dt);
	_gameManager.Update(dt);
}

void SimulationClient::End()
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	_gameManager.End();
	#ifdef ENABLE_SQLITE
	debugDb_.Close();
	#endif
}

void SimulationClient::Draw(sf::RenderTarget& renderTarget)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif

	_gameManager.Draw(renderTarget);
}


void SimulationClient::SetPlayerInput(const PlayerInput playerInput)
{
	const auto currentFrame = _gameManager.GetCurrentFrame();
	_gameManager.SetPlayerInput(_gameManager.GetPlayerNumber(), playerInput, currentFrame);
}

void SimulationClient::DrawImGui()
{
	const auto windowName = "Client " + std::to_string(static_cast<unsigned>(_clientId));
	bool show = true;
	ImGui::Begin(windowName.c_str(), &show, ImGuiWindowFlags_AlwaysAutoResize);
	if (_gameManager.GetPlayerNumber() == INVALID_PLAYER && ImGui::Button("Spawn Player"))
	{
		auto joinPacket = std::make_unique<JoinPacket>();
		const auto* clientIdPtr = reinterpret_cast<std::uint8_t*>(&_clientId);
		for (std::size_t i = 0; i < sizeof(_clientId); i++)
		{
			joinPacket->clientId[i] = clientIdPtr[i];
		}
		SendReliablePacket(std::move(joinPacket));
	}

	_gameManager.DrawImGui();
	if (_srtt > 0.0f)
	{
		ImGui::Text("SRTT: %f", _srtt);
		ImGui::Text("RTTVAR: %f", _rttvar);
		ImGui::Text("RTO: %f", _rto);
	}
	ImGui::End();
}

void SimulationClient::SendUnreliablePacket(std::unique_ptr<Packet> packet)
{
	_server.PutPacketInReceiveQueue(std::move(packet), true);
}

void SimulationClient::SendReliablePacket(std::unique_ptr<Packet> packet)
{
	_server.PutPacketInReceiveQueue(std::move(packet), false);
}

void SimulationClient::ReceivePacket(const Packet* packet)
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
}
