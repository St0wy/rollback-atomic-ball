// ReSharper disable CppClangTidyCppcoreguidelinesProTypeStaticCastDowncast
#pragma once

#include <chrono>
#include <memory>

#include <SFML/Network/Packet.hpp>

#include "game/game_globals.hpp"

namespace game
{
enum class PacketType : std::uint8_t
{
	Join = 0u,
	SpawnPlayer,
	Input,
	SpawnBullet,
	ValidateState,
	StartGame,
	JoinAck,
	LoseGame,
	Ping,
	None,
};

/**
 * \brief PhysicsState is the type of the physics state checksum
 */
using PhysicsState = std::uint16_t;

/**
 * \brief Packet is a interface that defines what a packet with a PacketType.
 */
struct Packet
{
	virtual ~Packet() = default;
	PacketType packetType = PacketType::None;
};

inline sf::Packet& operator<<(sf::Packet& packetReceived, Packet& packet)
{
	const auto packetType = static_cast<std::uint8_t>(packet.packetType);
	packetReceived << packetType;
	return packetReceived;
}

inline sf::Packet& operator>>(sf::Packet& packetReceived, Packet& packet)
{
	std::uint8_t packetType;
	packetReceived >> packetType;
	packet.packetType = static_cast<PacketType>(packetType);
	return packetReceived;
}

/**
 * \brief TypedPacket is a template class that sets the packetType of Packet automatically at construction with the given type.
 * \tparam Type is the PacketType of the packet
 */
template <PacketType Type>
struct TypedPacket : Packet
{
	TypedPacket() { packetType = Type; }
};

template <typename T, size_t N>
sf::Packet& operator<<(sf::Packet& packet, const std::array<T, N>& t)
{
	for (auto& tmp : t)
	{
		packet << tmp;
	}
	return packet;
}

template <typename T, size_t N>
sf::Packet& operator>>(sf::Packet& packet, std::array<T, N>& t)
{
	for (auto& tmp : t)
	{
		packet >> tmp;
	}
	return packet;
}

/**
 * \brief JoinPacket is a TCP Packet that is sent by a client to the server to join a game.
 */
struct JoinPacket final : TypedPacket<PacketType::Join>
{
	std::array<std::uint8_t, sizeof(ClientId)> clientId{};
	std::array<std::uint8_t, sizeof(unsigned long)> startTime{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const JoinPacket& joinPacket)
{
	return packet << joinPacket.clientId << joinPacket.startTime;
}

inline sf::Packet& operator>>(sf::Packet& packet, JoinPacket& joinPacket)
{
	return packet >> joinPacket.clientId >> joinPacket.startTime;
}

/**
 * \brief JoinAckPacket is a TCP Packet that is sent by the server to the client to answer a join packet
 */
struct JoinAckPacket final : TypedPacket<PacketType::JoinAck>
{
	std::array<std::uint8_t, sizeof(ClientId)> clientId{};
	std::array<std::uint8_t, sizeof(unsigned short)> udpPort{};
};


inline sf::Packet& operator<<(sf::Packet& packet, const JoinAckPacket& spawnPlayerPacket)
{
	return packet << spawnPlayerPacket.clientId << spawnPlayerPacket.udpPort;
}

inline sf::Packet& operator>>(sf::Packet& packet, JoinAckPacket& joinPacket)
{
	return packet >> joinPacket.clientId >> joinPacket.udpPort;
}

/**
 * \brief SpawnPlayerPacket is a TCP Packet sent by the server to all clients to notify of the spawn of a new player
 */
struct SpawnPlayerPacket final : TypedPacket<PacketType::SpawnPlayer>
{
	std::array<std::uint8_t, sizeof(ClientId)> clientId{};
	PlayerNumber playerNumber = INVALID_PLAYER;
	std::array<std::uint8_t, sizeof(core::Vec2f)> pos{};
	std::array<std::uint8_t, sizeof(core::Degree)> angle{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const SpawnPlayerPacket& spawnPlayerPacket)
{
	return packet << spawnPlayerPacket.clientId << spawnPlayerPacket.playerNumber <<
		spawnPlayerPacket.pos << spawnPlayerPacket.angle;
}

inline sf::Packet& operator>>(sf::Packet& packet, SpawnPlayerPacket& spawnPlayerPacket)
{
	return packet >> spawnPlayerPacket.clientId >> spawnPlayerPacket.playerNumber >>
		spawnPlayerPacket.pos >> spawnPlayerPacket.angle;
}

/**
 * \brief PlayerInputPacket is a UDP Packet sent by the player client and then replicated by the server to all clients to share the currentFrame
 * and all the previous ones player inputs.
 */
struct PlayerInputPacket final : TypedPacket<PacketType::Input>
{
	PlayerNumber playerNumber = INVALID_PLAYER;
	std::array<std::uint8_t, sizeof(Frame)> currentFrame{};
	std::array<std::uint8_t, MAX_INPUT_NMB> inputs{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const PlayerInputPacket& playerInputPacket)
{
	return packet << playerInputPacket.playerNumber <<
		playerInputPacket.currentFrame << playerInputPacket.inputs;
}

inline sf::Packet& operator>>(sf::Packet& packet, PlayerInputPacket& playerInputPacket)
{
	return packet >> playerInputPacket.playerNumber >>
		playerInputPacket.currentFrame >> playerInputPacket.inputs;
}

/**
 * \brief StartGamePacket is a TCP Packet send by the server to start a game at a given time.
 */
struct StartGamePacket final : TypedPacket<PacketType::StartGame> {};

/**
 * \brief ValidateFramePacket is an UDP packet that is sent by the server to validate the last physics state of the world.
 */
struct ValidateFramePacket final : TypedPacket<PacketType::ValidateState>
{
	std::array<std::uint8_t, sizeof(Frame)> newValidateFrame{};
	std::array<std::uint8_t, sizeof(PhysicsState) * MAX_PLAYER_NMB> physicsState{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const ValidateFramePacket& validateFramePacket)
{
	return packet << validateFramePacket.newValidateFrame << validateFramePacket.physicsState;
}

inline sf::Packet& operator>>(sf::Packet& packet, ValidateFramePacket& validateFramePacket)
{
	return packet >> validateFramePacket.newValidateFrame >> validateFramePacket.physicsState;
}

/**
 * \brief WinGamePacket is a TCP Packet sent by the server to notify the clients that a certain player has won.
 */
struct LoseGamePacket final : TypedPacket<PacketType::LoseGame>
{
	bool hasLost = true;
};

inline sf::Packet& operator<<(sf::Packet& packet, const LoseGamePacket& winGamePacket)
{
	return packet << winGamePacket.hasLost;
}

inline sf::Packet& operator>>(sf::Packet& packet, LoseGamePacket& winGamePacket)
{
	return packet >> winGamePacket.hasLost;
}

/**
 * \brief PingPacket is an UDP Packet sent by the client to the server and resend by the server to measure the RTT between the client and the server.
 */
struct PingPacket final : TypedPacket<PacketType::Ping>
{
	std::array<std::uint8_t, sizeof(unsigned long long)> time{};
	std::array<std::uint8_t, sizeof(ClientId)> clientId{};
};

inline sf::Packet& operator<<(sf::Packet& packet, const PingPacket& pingPacket)
{
	return packet << pingPacket.time << pingPacket.clientId;
}

inline sf::Packet& operator>>(sf::Packet& packet, PingPacket& pingPacket)
{
	return packet >> pingPacket.time >> pingPacket.clientId;
}

inline void GeneratePacket(sf::Packet& packet, Packet& sendingPacket)
{
	packet << sendingPacket;
	switch (sendingPacket.packetType)
	{
	case PacketType::Join:
	{
		const auto& packetTmp = static_cast<JoinPacket&>(sendingPacket);
		packet << packetTmp;
		break;
	}
	case PacketType::SpawnPlayer:
	{
		const auto& packetTmp = static_cast<SpawnPlayerPacket&>(sendingPacket);
		packet << packetTmp;
		break;
	}
	case PacketType::Input:
	{
		const auto& packetTmp = static_cast<PlayerInputPacket&>(sendingPacket);
		packet << packetTmp;
		break;
	}
	case PacketType::ValidateState:
	{
		const auto& packetTmp = static_cast<ValidateFramePacket&>(sendingPacket);
		packet << packetTmp;
		break;
	}
	case PacketType::StartGame:
	{
		break;
	}
	case PacketType::JoinAck:
	{
		const auto& packetTmp = static_cast<JoinAckPacket&>(sendingPacket);
		packet << packetTmp;
		break;
	}
	case PacketType::LoseGame:
	{
		const auto& packetTmp = static_cast<LoseGamePacket&>(sendingPacket);
		packet << packetTmp;
		break;
	}
	case PacketType::Ping:
	{
		const auto& packetTmp = static_cast<PingPacket&>(sendingPacket);
		packet << packetTmp;
		break;
	}
	default:
		break;
	}
}

inline std::unique_ptr<Packet> GenerateReceivedPacket(sf::Packet& packet)
{
	Packet packetTmp;
	packet >> packetTmp;
	switch (packetTmp.packetType)
	{
	case PacketType::Join:
	{
		auto joinPacket = std::make_unique<JoinPacket>();
		joinPacket->packetType = packetTmp.packetType;
		packet >> *joinPacket;
		return joinPacket;
	}
	case PacketType::SpawnPlayer:
	{
		auto spawnPlayerPacket = std::make_unique<SpawnPlayerPacket>();
		spawnPlayerPacket->packetType = packetTmp.packetType;
		packet >> *spawnPlayerPacket;
		return spawnPlayerPacket;
	}
	case PacketType::Input:
	{
		auto playerInputPacket = std::make_unique<PlayerInputPacket>();
		playerInputPacket->packetType = packetTmp.packetType;
		packet >> *playerInputPacket;
		return playerInputPacket;
	}
	case PacketType::ValidateState:
	{
		auto validateFramePacket = std::make_unique<ValidateFramePacket>();
		validateFramePacket->packetType = packetTmp.packetType;
		packet >> *validateFramePacket;
		return validateFramePacket;
	}
	case PacketType::StartGame:
	{
		auto startGamePacket = std::make_unique<StartGamePacket>();
		startGamePacket->packetType = packetTmp.packetType;
		return startGamePacket;
	}
	case PacketType::JoinAck:
	{
		auto joinAckPacket = std::make_unique<JoinAckPacket>();
		joinAckPacket->packetType = packetTmp.packetType;
		packet >> *joinAckPacket;
		return joinAckPacket;
	}
	case PacketType::LoseGame:
	{
		auto winGamePacket = std::make_unique<LoseGamePacket>();
		winGamePacket->packetType = packetTmp.packetType;
		packet >> *winGamePacket;
		return winGamePacket;
	}
	case PacketType::Ping:
	{
		auto pingPacket = std::make_unique<PingPacket>();
		pingPacket->packetType = packetTmp.packetType;
		packet >> *pingPacket;
		return pingPacket;
	}
	default: ;
	}
	return nullptr;
}

/**
 * \brief PacketSenderInterface is a interface for any Server or Client who wants to send and receive packets
 */
class PacketSenderInterface
{
public:
	virtual ~PacketSenderInterface() = default;
	virtual void SendReliablePacket(std::unique_ptr<Packet> packet) = 0;
	virtual void SendUnreliablePacket(std::unique_ptr<Packet> packet) = 0;
};
}
