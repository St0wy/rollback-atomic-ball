#include <imgui.h>

#include <maths/basic.hpp>

#include <network/simulation_client.hpp>
#include <network/simulation_server.hpp>

#include <utils/conversion.hpp>
#include <utils/log.hpp>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{
SimulationServer::SimulationServer(std::array<std::unique_ptr<SimulationClient>, 2>& clients) : _clients(clients)
{
}

void SimulationServer::Begin()
{

}

void SimulationServer::Update(const sf::Time dt)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto packetIt = _receivedPackets.begin();
    while (packetIt != _receivedPackets.end())
    {
        packetIt->currentTime -= dt.asSeconds();
        if (packetIt->currentTime <= 0.0f)
        {
            ProcessReceivePacket(std::move(packetIt->packet));

            packetIt = _receivedPackets.erase(packetIt);
        }
        else
        {
            ++packetIt;
        }

    }

    packetIt = _sentPackets.begin();
    while (packetIt != _sentPackets.end())
    {
        packetIt->currentTime -= dt.asSeconds();
        if (packetIt->currentTime <= 0.0f)
        {
            for (const auto& client : _clients)
            {
                client->ReceivePacket(packetIt->packet.get());
            }
            packetIt->packet = nullptr;
            packetIt = _sentPackets.erase(packetIt);
        }
        else
        {
            ++packetIt;
        }
    }
}

void SimulationServer::End()
{
}

void SimulationServer::DrawImGui()
{
    ImGui::Begin("Server");
    float minDelay = _avgDelay - _marginDelay;
    float maxDelay = _avgDelay + _marginDelay;
    bool hasDelayChanged = false;
    hasDelayChanged = hasDelayChanged || ImGui::SliderFloat("Min Delay", &minDelay, 0.01f, maxDelay);
    hasDelayChanged = hasDelayChanged || ImGui::SliderFloat("Max Delay", &maxDelay, minDelay, 1.0f);
    if (hasDelayChanged)
    {
        _avgDelay = (maxDelay + minDelay) / 2.0f;
        _marginDelay = (maxDelay - minDelay) / 2.0f;
    }
    ImGui::SliderFloat("Packet Loss", &_packetLoss, 0.0f, 1.0f);
    ImGui::End();
}

void SimulationServer::PutPacketInSendingQueue(std::unique_ptr<Packet> packet)
{
    _sentPackets.push_back({ _avgDelay + core::RandomRange(-_marginDelay, _marginDelay), std::move(packet) });
}

void SimulationServer::PutPacketInReceiveQueue(std::unique_ptr<Packet> packet, bool unreliable)
{
    if(unreliable)
    {
        //Packet loss implementation
        if(core::RandomRange(0.0f, 1.0f) < _packetLoss)
        {
            return;
        }
    }
    _receivedPackets.push_back({ _avgDelay + core::RandomRange(-_marginDelay, _marginDelay), std::move(packet) });
}

void SimulationServer::SendReliablePacket(std::unique_ptr<Packet> packet)
{
    PutPacketInSendingQueue(std::move(packet));
}

void SimulationServer::SendUnreliablePacket(std::unique_ptr<Packet> packet)
{
    PutPacketInSendingQueue(std::move(packet));
}

void SimulationServer::ProcessReceivePacket(std::unique_ptr<Packet> packet)
{
    Server::ReceivePacket(std::move(packet));
}

void SimulationServer::SpawnNewPlayer(const ClientId clientId, const PlayerNumber playerNumber)
{
    core::LogInfo("[Server] Spawn new player");
    auto spawnPlayer = std::make_unique<SpawnPlayerPacket>();
    spawnPlayer->packetType = PacketType::SpawnPlayer;
    spawnPlayer->clientId = core::ConvertToBinary(clientId);
    spawnPlayer->playerNumber = playerNumber;

    const auto pos = SPAWN_POSITIONS[playerNumber] * 3.0f;
    spawnPlayer->pos = ConvertToBinary(pos);
    const auto rotation = SPAWN_ROTATIONS[playerNumber];
    spawnPlayer->angle = core::ConvertToBinary(rotation);
    _gameManager.SpawnPlayer(playerNumber, pos, rotation);
    SendReliablePacket(std::move(spawnPlayer));
}
}
