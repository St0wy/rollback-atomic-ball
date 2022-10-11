#pragma once
#include <memory>

#include <SFML/System/Time.hpp>

#include "debug_db.hpp"
#include "server.hpp"

#include "graphics/graphics.hpp"

namespace game
{
/**
 * \brief DelayPacket is a struct used by the SimulationServer to delay received Packet.
 */
struct DelayPacket
{
	float currentTime = 0.0f;
	std::unique_ptr<Packet> packet = nullptr;
};

class SimulationClient;

/**
 * \brief SimulationServer is a Server that delays Packet internally before "receiving" them and then sends them back with delay to the SimulationClient.
 */
class SimulationServer final : public Server, public core::DrawImGuiInterface
{
public:
	explicit SimulationServer(std::array<std::unique_ptr<SimulationClient>, 2>& clients);
	void Begin() override;
	void Update(sf::Time dt) override;
	void End() override;
	void DrawImGui() override;
	void PutPacketInReceiveQueue(std::unique_ptr<Packet> packet, bool unreliable);
	void SendReliablePacket(std::unique_ptr<Packet> packet) override;
	void SendUnreliablePacket(std::unique_ptr<Packet> packet) override;
private:
	void PutPacketInSendingQueue(std::unique_ptr<Packet> packet);
	void ProcessReceivePacket(std::unique_ptr<Packet> packet);

	void SpawnNewPlayer(ClientId clientId, PlayerNumber playerNumber) override;

	std::vector<DelayPacket> _receivedPackets;
	std::vector<DelayPacket> _sentPackets;
	std::array<std::unique_ptr<SimulationClient>, MAX_PLAYER_NMB>& _clients;
	float _avgDelay = 0.25f;
	float _marginDelay = 0.1f;
	float _packetLoss = 0.0f;
};
}
