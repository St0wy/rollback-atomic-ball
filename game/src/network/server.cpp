#include <cstdint>

#include <network/server.hpp>

#include <utils/conversion.hpp>
#include <utils/log.hpp>

#include "maths/basic.hpp"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
void Server::SendStartGamePacket()
{
	auto startGamePacket = std::make_unique<StartGamePacket>();
	startGamePacket->packetType = PacketType::StartGame;
	core::LogInfo("Send Start Game Packet");
	SendReliablePacket(std::move(startGamePacket));
}

void Server::SendSpawnFallingWallPacket(const Frame spawnTimeOffset)
{
	auto spawnFallingWallPacket = std::make_unique<SpawnFallingWallPacket>();
	spawnFallingWallPacket->packetType = PacketType::SpawnFallingWall;


	const Frame currentFrame = _gameManager.GetRollbackManager().GetCurrentFrame();
	// We want more time on the first wall

	const Frame offsetFrame = GetNextRandomFallingWallSpawnFrame() + spawnTimeOffset;

	FallingWallSpawnInstructions fallingWallSpawnInstructions{};
	fallingWallSpawnInstructions.spawnFrame = currentFrame + offsetFrame;
	fallingWallSpawnInstructions.requiresBall = core::RandomBool();
	fallingWallSpawnInstructions.doorPosition = GetNextRandomDoorPosition();

	_gameManager.SetFallingWallSpawnInstructions(fallingWallSpawnInstructions);

	spawnFallingWallPacket->spawnFrame = core::ConvertToBinary(fallingWallSpawnInstructions.spawnFrame);
	spawnFallingWallPacket->requiresBall = fallingWallSpawnInstructions.requiresBall;
	spawnFallingWallPacket->doorPosition = core::ConvertToBinary(fallingWallSpawnInstructions.doorPosition);

	core::LogInfo("Send Spawn Wall Packet");

	SendReliablePacket(std::move(spawnFallingWallPacket));
}

Frame Server::GetNextRandomFallingWallSpawnFrame() const
{
	return core::RandomRange<Frame>(10u, 50u);
}

float Server::GetNextRandomDoorPosition() const
{
	// Check if the door will be on the left (true) or on the right
	if (core::RandomBool())
	{
		return core::RandomRange<float>(-10.0f, -3.0f);
	}

	return core::RandomRange<float>(3.0f, 10.0f);
}

void Server::ReceivePacket(std::unique_ptr<Packet> packet)
{
	#ifdef TRACY_ENABLE
	ZoneScoped;
	#endif
	switch (packet->packetType)
	{
	case PacketType::Join:
	{
		const auto* joinPacket = dynamic_cast<const JoinPacket*>(packet.get());
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
			SendStartGamePacket();
			SendSpawnFallingWallPacket(500u);
		}

		break;
	}
	case PacketType::Input:
	{
		// Manage internal state
		const auto* playerInputPacket = dynamic_cast<const PlayerInputPacket*>(packet.get());
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

		// Validate new frame if needed
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
			// Validate frame
			_gameManager.Validate(lastReceiveFrame);

			auto validateFramePacket = std::make_unique<ValidateFramePacket>();
			validateFramePacket->newValidateFrame = core::ConvertToBinary(lastReceiveFrame);

			// copy physics state
			for (PlayerNumber i = 0; i < MAX_PLAYER_NMB; i++)
			{
				auto physicsState = _gameManager.GetRollbackManager().GetValidatePhysicsState(i);
				const auto* statePtr = reinterpret_cast<const std::uint8_t*>(&physicsState);
				for (size_t j = 0; j < sizeof(PhysicsState); j++)
				{
					validateFramePacket->physicsState[i * sizeof(PhysicsState) + j] = statePtr[j];
				}
			}

			SendUnreliablePacket(std::move(validateFramePacket));

			// TODO : Check for spawning falling wall

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
