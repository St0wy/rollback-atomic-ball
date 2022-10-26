#include <cstdint>

#include <network/server.hpp>

#include <utils/conversion.hpp>
#include <utils/log.hpp>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
void Server::ReceivePacket(std::unique_ptr<Packet> packet)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	switch (packet->packetType)
	{
	case PacketType::Join:
	{
		const auto* joinPacket = static_cast<const JoinPacket*>(packet.get());
		const auto clientId = core::ConvertFromBinary<ClientId>(joinPacket->clientId);
		if (std::ranges::any_of(_clientMap,
			[clientId](const auto clientMapId)
		{
			return clientMapId == clientId;
		}))
		{
			//Player joined twice!
			return;
		}
		core::LogInfo("Managing Received Packet Join from: " + std::to_string(static_cast<unsigned>(clientId)));
		_clientMap[_lastPlayerNumber] = clientId;
		SpawnNewPlayer(clientId, _lastPlayerNumber);

		_lastPlayerNumber++;

		if (_lastPlayerNumber == MAX_PLAYER_NMB)
		{
			auto startGamePacket = std::make_unique<StartGamePacket>();
			startGamePacket->packetType = PacketType::StartGame;
			core::LogInfo("Send Start Game Packet");
			SendReliablePacket(std::move(startGamePacket));
		}

		break;
	}
	case PacketType::Input:
	{
		//Manage internal state
		const auto* playerInputPacket = static_cast<const PlayerInputPacket*>(packet.get());
		const auto playerNumber = playerInputPacket->playerNumber;
		const auto inputFrame = core::ConvertFromBinary<Frame>(playerInputPacket->currentFrame);

		for (std::uint32_t i = 0; i < playerInputPacket->inputs.size(); i++)
		{
			_gameManager.SetPlayerInput(playerNumber,
				playerInputPacket->inputs[i],
				inputFrame - i);
			if (inputFrame - i == 0)
			{
				break;
			}
		}

		SendUnreliablePacket(std::move(packet));

		//Validate new frame if needed
		std::uint32_t lastReceiveFrame = _gameManager.GetRollbackManager().GetLastReceivedFrame(0);
		for (PlayerNumber i = 1; i < MAX_PLAYER_NMB; i++)
		{
			const auto playerLastFrame = _gameManager.GetRollbackManager().GetLastReceivedFrame(i);
			if (playerLastFrame < lastReceiveFrame)
			{
				lastReceiveFrame = playerLastFrame;
			}
		}
		if (lastReceiveFrame > _gameManager.GetLastValidateFrame())
		{
			//Validate frame
			_gameManager.Validate(lastReceiveFrame);

			auto validatePacket = std::make_unique<ValidateFramePacket>();
			validatePacket->newValidateFrame = core::ConvertToBinary(lastReceiveFrame);

			//copy physics state
			for (PlayerNumber i = 0; i < MAX_PLAYER_NMB; i++)
			{
				auto physicsState = _gameManager.GetRollbackManager().GetValidatePhysicsState(i);
				const auto* statePtr = reinterpret_cast<const std::uint8_t*>(&physicsState);
				for (size_t j = 0; j < sizeof(PhysicsState); j++)
				{
					validatePacket->physicsState[i * sizeof(PhysicsState) + j] = statePtr[j];
				}
			}
			SendUnreliablePacket(std::move(validatePacket));
			if (_gameManager.CheckIfLost())
			{
				core::LogInfo("Server declares everyone lost");
				auto winGamePacket = std::make_unique<LoseGamePacket>();
				winGamePacket->hasLost = true;
				SendReliablePacket(std::move(winGamePacket));
				_gameManager.LoseGame();
			}
		}

		break;
	}
	case PacketType::Ping:
	{
		auto pingPacket = std::make_unique<PingPacket>();
		*pingPacket = *static_cast<PingPacket*>(packet.get());
		SendUnreliablePacket(std::move(pingPacket));
		break;
	}
	default:
		break;
	}
}
}
