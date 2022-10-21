// ReSharper disable CppClangTidyCppcoreguidelinesProTypeStaticCastDowncast
#include "network/client.hpp"

#include "maths/basic.hpp"

#include "utils/assert.hpp"
#include "utils/conversion.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
void Client::ReceivePacket(const Packet* packet)
{
	#ifdef TRACY_ENABLE
    ZoneScoped;
	#endif

	switch (packet->packetType)
	{
	case PacketType::SpawnPlayer:
	{
		const auto* spawnPlayerPacket = static_cast<const SpawnPlayerPacket*>(packet);
		const auto clientId = core::ConvertFromBinary<ClientId>(spawnPlayerPacket->clientId);

		const PlayerNumber playerNumber = spawnPlayerPacket->playerNumber;
		if (clientId == _clientId)
		{
			_gameManager.SetClientPlayer(playerNumber);
		}

		const auto pos = core::ConvertFromBinary<core::Vec2f>(spawnPlayerPacket->pos);
		const auto rotation = core::ConvertFromBinary<core::Degree>(spawnPlayerPacket->angle);

		_gameManager.SpawnPlayer(playerNumber, pos, rotation);
		break;
	}
	case PacketType::StartGame:
	{
		core::LogInfo("Start Game Packet Received");
		using namespace std::chrono;
		const auto startingTime = (duration_cast<duration<long long, std::milli>>(
			system_clock::now().time_since_epoch()
		) + milliseconds(START_DELAY)).count() - milliseconds(static_cast<long long>(_currentPing)).count();

		_gameManager.StartGame(startingTime);
		break;
	}
	case PacketType::Input:
	{
		const auto* playerInputPacket = static_cast<const PlayerInputPacket*>(packet);
		const auto playerNumber = playerInputPacket->playerNumber;
		const auto inputFrame = core::ConvertFromBinary<Frame>(playerInputPacket->currentFrame);

		if (playerNumber == _gameManager.GetPlayerNumber())
		{
			//Verify the inputs coming back from the server
			const auto& inputs = _gameManager.GetRollbackManager().GetInputs(playerNumber);
			const auto currentFrame = _gameManager.GetRollbackManager().GetCurrentFrame();
			for (size_t i = 0; i < playerInputPacket->inputs.size(); i++)
			{
				const auto index = static_cast<unsigned long long>(currentFrame) - inputFrame + i;
				if (index > inputs.size())
				{
					break;
				}
				if (inputs[index] != playerInputPacket->inputs[i])
				{
					gpr_assert(false, "Inputs coming back from server are not coherent!!!");
				}
				if (inputFrame - i == 0)
				{
					break;
				}
			}
			break;
		}

		//discard delayed input packet
		if (inputFrame < _gameManager.GetRollbackManager().GetLastReceivedFrame(playerNumber))
		{
			break;
		}
		for (Frame i = 0; i < playerInputPacket->inputs.size(); i++)
		{
			_gameManager.SetPlayerInput(playerNumber,
				playerInputPacket->inputs[i],
				inputFrame - i);

			if (inputFrame - i == 0)
			{
				break;
			}
		}
		break;
	}
	case PacketType::ValidateState:
	{
		const auto* validateFramePacket = static_cast<const ValidateFramePacket*>(packet);
		const auto newValidateFrame = core::ConvertFromBinary<Frame>(validateFramePacket->newValidateFrame);
		std::array<PhysicsState, MAX_PLAYER_NMB> physicsStates{};
		for (size_t i = 0; i < validateFramePacket->physicsState.size(); i++)
		{
			auto* statePtr = reinterpret_cast<std::uint8_t*>(physicsStates.data());
			statePtr[i] = validateFramePacket->physicsState[i];
		}
		_gameManager.ConfirmValidateFrame(newValidateFrame, physicsStates);
		//logDebug("Client received validate frame " + std::to_string(newValidateFrame));
		break;
	}
	case PacketType::WinGame:
	{
		const auto* winGamePacket = static_cast<const WinGamePacket*>(packet);
		_gameManager.WinGame(winGamePacket->winner);
		break;
	}
	case PacketType::Ping:
	{
		const auto* pingPacket = static_cast<const PingPacket*>(packet);
		const auto clientId = core::ConvertFromBinary<ClientId>(pingPacket->clientId);
		if (clientId == _clientId)
		{
			const auto originTime = core::ConvertFromBinary<unsigned long long>(pingPacket->time);
			using namespace std::chrono;
			const auto currentTime = duration_cast<duration<unsigned long long, std::milli>>(
				system_clock::now().time_since_epoch()
			).count();
			const auto delta = currentTime - originTime;
			const auto ping = static_cast<float>(delta);

			//calculate average and var ping
			if (_srtt < 0.0f)
			{
				_srtt = ping;
				_rttvar = ping / 2.0f;
			}
			else
			{
				_srtt = (1.0f - ALPHA) * _srtt + ALPHA * ping;
				_rttvar = (1.0f - BETA) * _rttvar + BETA * core::Abs(_srtt - ping);
			}

			_rto = _srtt + std::max(G, K * _rttvar);
			_currentPing = _srtt;
		}
		break;
	}
	default: 
		break;
	}
}

void Client::Update(const sf::Time dt)
{
	#ifdef TRACY_ENABLE
    ZoneScoped;

	#endif
	_pingTimer -= dt.asSeconds();
	if (_pingTimer < 0.0f)
	{
		if (_clientId != INVALID_CLIENT_ID)
		{
			using namespace std::chrono;
			auto pingPacket = std::make_unique<PingPacket>();
			pingPacket->time = core::ConvertToBinary(duration_cast<duration<unsigned long long, std::milli>>(
				system_clock::now().time_since_epoch()).count());
			pingPacket->clientId = core::ConvertToBinary(_clientId);
			SendUnreliablePacket(std::move(pingPacket));
		}
		_pingTimer = PING_PERIOD_;
	}
}
}
