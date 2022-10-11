#pragma once

#include <SFML/Window/Event.hpp>

#include "engine/app.hpp"

#include "network/network_client.hpp"

namespace game
{
/**
 * \brief ClientApp is a App that owns a NetworkClient and shows it on the screen directly.
 */
class ClientApp final : public core::App
{
public:
    void Begin() override;

    void Update(sf::Time dt) override;

    void End() override;

    void DrawImGui() override;

    void OnEvent(const sf::Event& event) override;

    void Draw(sf::RenderTarget& window) override
    {
        _client.Draw(window);
    }

private:
    sf::Vector2u _windowSize;
    NetworkClient _client;
};
} // namespace game
