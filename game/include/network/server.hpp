#pragma once
#include <memory>

#include "packet_type.hpp"

#include "engine/system.hpp"

#include "game/game_globals.hpp"
#include "game/game_manager.hpp"

namespace game
{
/**
 * \brief Server is an interface to a network or simulated server. It owns a simple GameManager.
 */
class Server : public PacketSenderInterface, public core::SystemInterface
{
protected:

    virtual void SpawnNewPlayer(ClientId clientId, PlayerNumber playerNumber) = 0;

    /**
     * \brief ReceiveNetPacket is a method that is called when the Server receives a Packet from a Client.
     * \param packet is the received Packet.
     */
    virtual void ReceivePacket(std::unique_ptr<Packet> packet);

    //Server game manager
    GameManager _gameManager;
    PlayerNumber _lastPlayerNumber = 0;
    std::array<ClientId, MAX_PLAYER_NMB> _clientMap{};

};
}
